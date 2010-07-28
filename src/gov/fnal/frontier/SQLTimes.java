/**
 * Manage modification times for SQL
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import gov.fnal.frontier.plugin.*;
import java.sql.*;
import java.util.Calendar;
import java.text.SimpleDateFormat;
import java.util.HashSet;
import java.util.Iterator;

class SQLTimes
 {
  // need separate SimpleDateFormat object for each SQLTimes to simplify
  //  synchronization; it isn't thread safe on its own
  private SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yyyy HH:mm:ss");
  private long last_modified;
  private long last_validated;

  private HashSet<String> queryTableNames;

  private static final String timestamp_sql1="select to_char(change_time,'mm/dd/yyyy hh24:mi:ss') from ";
  private static final String timestamp_sql1_max="select max(to_char(change_time,'mm/dd/yyyy hh24:mi:ss')) from ";
  private static final String tableAndOwner=" UPPER(table_owner)=? and UPPER(table_name)=?";
  private static final String timestamp_sql2=" where "+tableAndOwner;
  private static final String timestamp_sql2_or=" or ("+tableAndOwner+")";

  public SQLTimes(HashSet<String> qTableNames)
   {
    last_modified=0;
    last_validated=0;
    queryTableNames=qTableNames;
   }

  public String getTableOwner(String tableName)
   {
    String tableOwner;
    int dotIndex=tableName.indexOf('.');
    tableOwner=tableName.substring(0,dotIndex);
    // remove any quotes from each part
    // if there weren't any quotes, make it upper case
    char firstchar=tableOwner.charAt(0);
    if((firstchar=='"')||(firstchar=='\''))
     tableOwner=tableOwner.substring(1,tableOwner.length()-1);
    else
      tableOwner=tableOwner.toUpperCase();
    return tableOwner;
   }

  public String getObjectName(String tableName)
   {
    String objectName;
    int dotIndex=tableName.indexOf('.');
    objectName=tableName.substring(dotIndex+1);
    // remove any quotes from each part
    // if there weren't any quotes, make it upper case
    char firstchar=objectName.charAt(0);
    if((firstchar=='"')||(firstchar=='\''))
     objectName=objectName.substring(1,objectName.length()-1);
    else
      objectName=objectName.toUpperCase();
    return objectName;
   }

  public synchronized long getCachedLastModified() throws Exception
   {
    //Return cached value of last-modified timestamp for this owner+table,
    //  if it is still valid, otherwise return 0.
    // Synchronized with getLastModified to prevent races between threads,
    //  potentially causing last_validated & last_modified to be inconsistent
    //  with each other.
    if((Calendar.getInstance().getTimeInMillis()-last_validated)>(Frontier.validate_last_modified_seconds*1000))
      // hasn't been validated recently enough, say it isn't cached
      return 0;
    return last_modified;
   }

  private PreparedStatement prepareStatement(java.sql.Connection con) throws Exception
   {
    PreparedStatement stmt=null;
    String timequery="";
    String sql1=timestamp_sql1_max;
    if (queryTableNames.size()==1) 
      sql1=timestamp_sql1;
    timequery=sql1+Frontier.last_modified_table_name+timestamp_sql2;
    Iterator it=queryTableNames.iterator();
    it.next(); // To prevent referring twice to the first table
    while (it.hasNext()) 
     {
      String tableName=(String)it.next();
      timequery=timequery+timestamp_sql2_or;
     }
    if(Frontier.getHighVerbosity())Frontier.Log("timequery: "+timequery); 

    stmt=con.prepareStatement(timequery);

    it=queryTableNames.iterator(); // init the iterator
    int i=1;
    while (it.hasNext())
     {
      String tableName=(String)it.next();
      String owner=getTableOwner(tableName).toUpperCase();
      String object=getObjectName(tableName).toUpperCase();
      stmt.setString(i,owner);i++;
      if(Frontier.getHighVerbosity())Frontier.Log("tableName: "+tableName+", owner: "+owner+", object: "+object);
      stmt.setString(i,object);i++;
     } 

    return stmt;
   }

  public synchronized long getLastModified(java.sql.Connection con) throws Exception
   {
    if(Frontier.getHighVerbosity())Frontier.Log("getLastModified: start");
    //first check, now that we've acquired the database, if in the meantime
    // another thread has gotten through and validated the information
    if(getCachedLastModified()!=0)
     {
      Frontier.Log("another thread found last-modified time, using cached value");
      return last_modified;
     }

    //read the last-modified timestamp for this owner+table from the database
    PreparedStatement stmt=null;
    ResultSet rs=null;
    String stamp=null;
    // can't pass the table name as bind variable, splice it in
    try
     {
      stmt=prepareStatement(con);
      try
       {
        rs=stmt.executeQuery();
       }
      catch(Exception e)
       {
        Exception newe=new Exception("Error querying for "+queryTableNames.toString()+" in "+Frontier.last_modified_table_name+": "+e.getMessage().trim());
	newe.setStackTrace(e.getStackTrace());
	throw newe;
       }
      if(!rs.next())
       {
        throw new Exception(queryTableNames.toString()+" not found in "+Frontier.last_modified_table_name);
       }
      else
       {
        stamp=rs.getString(1);
        last_modified=date_fmt.parse(stamp).getTime(); 
       }
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
     }

    last_validated=Calendar.getInstance().getTimeInMillis();

    if(Frontier.getHighVerbosity())Frontier.Log("getLastModified: last_validated: "+last_validated+", last_modified: "+last_modified+": "+stamp);
    return last_modified;
  }
 }
