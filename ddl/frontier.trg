-- D:\mydocs\docs\desinger\frontier.trg
--
-- Generated for Oracle 9i on Tue Aug 24  13:54:32 2004 by Server Generator 9.0.2.92.10
PROMPT Creating Trigger 'FD_PREUPDATE_TRG'
CREATE OR REPLACE TRIGGER FD_PREUPDATE_TRG
 BEFORE UPDATE
 ON FRONTIER_DESCRIPTORS
 FOR EACH ROW
begin
	:new.update_date := sysdate;
	:new.update_user := lower(user);
end;
/
SHOW ERROR


PROMPT Creating Trigger 'FD_PREINSERT_TRG'
CREATE OR REPLACE TRIGGER FD_PREINSERT_TRG
 BEFORE INSERT
 ON FRONTIER_DESCRIPTORS
 FOR EACH ROW
begin
	:new.create_date := sysdate;
	:new.create_user := lower(user);
end;
/
SHOW ERROR
