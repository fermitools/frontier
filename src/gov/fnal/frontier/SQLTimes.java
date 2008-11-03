package gov.fnal.frontier;

import gov.fnal.frontier.plugin.*;
import java.sql.*;
import java.util.Calendar;
import java.text.SimpleDateFormat;

class SQLTimes
 {
  // need separate SimpleDateFormat object for each SQLTimes to simplify
  //  synchronization; it isn't thread safe on its own
  private SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yyyy HH:mm:ss");
  private long last_modified;
  private long last_validated;
  private String queryTableOwner;
  private String queryTableObjectName;
  private static final String timestamp_sql1="select to_char(change_time, 'mm/dd/yyyy hh24:mi:ss') as our_time from ";
  private static final String timestamp_sql2=".last_modified_times where table_name=?";

  public SQLTimes(String tableName)
   {
    last_modified=0;
    last_validated=0;
    int dotIndex=tableName.indexOf('.');
    queryTableOwner=tableName.substring(0,dotIndex);
    queryTableObjectName=tableName.substring(dotIndex+1);
    // remove any quotes from the table object part
    // if there weren't any quotes, make it upper case
    char firstchar=queryTableObjectName.charAt(0);
    if((firstchar=='"')||(firstchar=='\''))
     queryTableObjectName=queryTableObjectName.substring(1,queryTableObjectName.length()-1);
    else
      queryTableObjectName=queryTableObjectName.toUpperCase();
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

  public synchronized long getLastModified(java.sql.Connection con) throws Exception
   {
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
    // can't pass the queryTableOwner as bind variable, splice it in
    String timequery=timestamp_sql1+queryTableOwner+timestamp_sql2;
    try
     {
      stmt=con.prepareStatement(timequery);
      stmt.setString(1,queryTableObjectName);
      String queryTableParam="where ? = "+queryTableObjectName;
      try
       {
        rs=stmt.executeQuery();
       }
      catch(Exception e)
       {
        Exception newe=new Exception("Error querying for "+queryTableObjectName+" from "+queryTableOwner+".LAST_MODIFIED_TIMES: "+e.getMessage().trim());
	newe.setStackTrace(e.getStackTrace());
	throw newe;
       }
      if(!rs.next())
       {
        throw new Exception(queryTableObjectName+" not found in "+queryTableOwner+".LAST_MODIFIED_TIMES");
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

    return last_modified;
  }
 }
