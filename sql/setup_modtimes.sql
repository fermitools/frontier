/* This script initializes table "last_modified_times" if it doesn't exist   */
/*  and creates dbms_change_notification callback requests on every other    */
/*  table in the current account.  The table is needed for the quick-update  */
/*  frontier caching mechanism.  The callbacks update a timestamp in         */
/*  last_modified_times whenever any other table in the current account      */
/*  changes.  This script also creates a trigger on dropping tables that     */
/*  automatically deletes the corresponding last_modified_times entry and    */
/*  callback request, and a trigger on creating tables that automatically    */
/*  adds the entry and creates the callback request to keep the entry up to  */
/*  date.  Even if all tables are dropped, the create trigger will re-create */
/*  "last_modified_times" and keep it up to date so this script should only  */
/*  have to be run once for each account.  An additional 'ALL_TABLES' entry  */
/*  timestamp is updated whenever any new table is created or dropped.       */
/* Written by Dave Dykstra, July 2008.                                       */
/* $Id$
 */


CREATE OR REPLACE PROCEDURE chtable_callback(ntfnds IN SYS.CHNF$_DESC) AS
  event_type	NUMBER;
  numtables	NUMBER;
  tdesc		SYS.CHNF$_TDESC;
  tabname	all_tables.table_name%TYPE;
BEGIN
  numtables  := ntfnds.numtables;
  event_type := ntfnds.event_type;

  IF (event_type = DBMS_CHANGE_NOTIFICATION.EVENT_OBJCHANGE) THEN
    FOR i IN 1..numtables LOOP
      tdesc := ntfnds.table_desc_array(i);
      tabname := SUBSTR(tdesc.table_name, INSTR(tdesc.table_name, '.') + 1);
      IF (bitand(tdesc.opflags, DBMS_CHANGE_NOTIFICATION.DROPOP) = 0) THEN
        EXECUTE IMMEDIATE 'UPDATE last_modified_times SET change_time = sysdate WHERE table_name = '''||tabname||'''';
      END IF;
    END LOOP;
    COMMIT;
  END IF;
END;
/

CREATE OR REPLACE PROCEDURE do_unreg_table(tname all_tables.table_name%TYPE)
    AUTHID CURRENT_USER AS
  thisschema	all_tables.owner%TYPE;
  CURSOR reg_cursor(regname user_change_notification_regs.table_name%TYPE) IS
    SELECT regid FROM user_change_notification_regs WHERE table_name=regname;
BEGIN
  /* delete all old registrations for this table if any exist */
  thisschema := SYS_CONTEXT('USERENV','CURRENT_SCHEMA');
  FOR idx IN reg_cursor(thisschema||'.'||tname) LOOP
    DBMS_CHANGE_NOTIFICATION.DEREGISTER (idx.regid);
  END LOOP;
END;
/

CREATE OR REPLACE PROCEDURE do_update_all_tables
    AUTHID CURRENT_USER AS
BEGIN
  EXECUTE IMMEDIATE 'UPDATE last_modified_times SET change_time = SYSDATE WHERE table_name = ''ALL_TABLES''';
  IF SQL%ROWCOUNT = 0 THEN
    /* insert it if it didn't exist */
    EXECUTE IMMEDIATE 'INSERT INTO last_modified_times VALUES(''ALL_TABLES'', SYSDATE)';
  END IF;
END;
/

CREATE OR REPLACE PROCEDURE do_new_table(tname all_tables.table_name%TYPE)
    AUTHID CURRENT_USER AS
  regds		SYS.CHNF$_REG_INFO;
  regid		NUMBER;
  nummodentries	NUMBER;
  tabname	all_tables.table_name%TYPE;
  CURSOR lastmod_cursor IS
    SELECT table_name FROM user_tables WHERE table_name='LAST_MODIFIED_TIMES';
BEGIN
  /* create last_modified_times if it doesn't exist */
  OPEN lastmod_cursor;
  FETCH lastmod_cursor INTO tabname;
  IF lastmod_cursor%NOTFOUND THEN
    EXECUTE IMMEDIATE 'CREATE TABLE last_modified_times(table_name VARCHAR2(100), change_time Date, CONSTRAINT last_modified_times_pk PRIMARY KEY (table_name))';
    EXECUTE IMMEDIATE 'GRANT select ON last_modified_times TO public';
  END IF;
  CLOSE lastmod_cursor;

  /* if last_modified_times entry for table doesn't exist, insert it */
  EXECUTE IMMEDIATE 'SELECT COUNT(*) FROM last_modified_times WHERE table_name = '''||tname||'''' INTO nummodentries;
  IF nummodentries = 0 THEN
    EXECUTE IMMEDIATE 'INSERT INTO last_modified_times VALUES('''||tname||''', SYSDATE)';
    do_update_all_tables();
  END IF;
  COMMIT; /* have to commit before registering change notification */

  /* unregister previous callbacks if any */
  do_unreg_table(tname);

  /* register a callback for updating the entry when the table is modified */
  regds := SYS.CHNF$_REG_INFO ('chtable_callback', 0, 0, 0, 0);
  regid := DBMS_CHANGE_NOTIFICATION.NEW_REG_START (regds);
  EXECUTE IMMEDIATE 'SELECT COUNT(*) FROM ' || tname;
  DBMS_CHANGE_NOTIFICATION.REG_END;

END;
/

CREATE OR REPLACE PROCEDURE do_create_trigger(tname all_tables.table_name%TYPE)
    AUTHID CURRENT_USER AS
  PRAGMA AUTONOMOUS_TRANSACTION;
BEGIN
  /* delay the processing because otherwise get a deadlock during change
     notification registration because table not yet completely set up;
     it requires manually running DBMS_CHANGE_NOTIFICATION.REG_END to 
     recover */
  DBMS_SCHEDULER.CREATE_JOB(
    job_name => DBMS_SCHEDULER.GENERATE_JOB_NAME('CREATETRIG$_'),
    job_type => 'PLSQL_BLOCK',
    job_action => 'BEGIN do_new_table('''||tname||'''); END;',
    enabled => true);
END;
/

CREATE OR REPLACE PROCEDURE do_drop_trigger(tname all_tables.table_name%TYPE)
    AUTHID CURRENT_USER AS
  tabname	all_tables.table_name%TYPE;
  CURSOR lastmod_cursor IS
    SELECT table_name FROM user_tables WHERE table_name='LAST_MODIFIED_TIMES';
BEGIN
  /* unregister any callbacks for this table */
  do_unreg_table(tname);

  /* drop entry from last_modified_times if that table exists */
  OPEN lastmod_cursor;
  FETCH lastmod_cursor INTO tabname;
  IF lastmod_cursor%FOUND THEN
    EXECUTE IMMEDIATE 'DELETE FROM last_modified_times WHERE table_name = '''||tname||'''';
    do_update_all_tables();
    /* can't COMMIT this because in a trigger; could do it with PRAGMA
       AUTONOMOUS_TRANSACTION but a ROLLBACK doesn't affect it anyway */
  END IF;
  CLOSE lastmod_cursor;
END;
/

DECLARE
  CURSOR trigger_names IS
    SELECT trigger_name FROM user_triggers;
  CURSOR table_names IS
    SELECT table_name FROM user_tables;
BEGIN
  /* treat each existing table except last_modified_times like a new table */
  FOR idx IN table_names LOOP
    IF idx.table_name != 'LAST_MODIFIED_TIMES' THEN
      do_new_table(idx.table_name);
    END IF;
  END LOOP;

  /* create schema-level trigger to register callback when new table created */
  EXECUTE IMMEDIATE 'CREATE OR REPLACE TRIGGER LASTMOD_CREATE'||
    ' AFTER CREATE ON SCHEMA '||
    'BEGIN '||
      'IF ora_dict_obj_type = ''TABLE'' THEN '||
	'IF ora_dict_obj_name != ''LAST_MODIFIED_TIMES'' THEN '||
          'do_create_trigger(ora_dict_obj_name); '||
        'END IF;'||
      'END IF;'||
    'END;';

  /* create schema-level trigger to unregister callback when table dropped */
  EXECUTE IMMEDIATE 'CREATE OR REPLACE TRIGGER LASTMOD_DROP'||
    ' AFTER DROP ON SCHEMA '||
    'BEGIN '||
      'IF ora_dict_obj_type = ''TABLE'' THEN '||
	'IF ora_dict_obj_name != ''LAST_MODIFIED_TIMES'' THEN '||
          'do_drop_trigger(ora_dict_obj_name); '||
        'END IF;'||
      'END IF;'||
    'END;';
END;
/
