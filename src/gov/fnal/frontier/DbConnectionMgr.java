package gov.fnal.frontier;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.sql.Connection;
import java.sql.SQLException;
import javax.sql.DataSource;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.Timer;
import java.util.TimerTask;
import javax.servlet.ServletOutputStream;

/**
 * Singleton class which provides database connections.  The specific
 * database supported is determined by configuring the server.xml file.
 * @author Stephen P. White <swhite@fnal.gov>
 * udapted for Frontier3 by: Sergey Kosyakov
 * @version $Revision$
 */
public class DbConnectionMgr 
 {
  private static DbConnectionMgr instance=null;
  private static Boolean mutex=new Boolean(true);
  private DataSource dataSource=null;

  public static DbConnectionMgr getDbConnectionMgr() throws Exception
   {
    if(instance!=null) return instance;
    
    synchronized(mutex)
     {
      if(instance!=null) return instance;      
      DbConnectionMgr m=new DbConnectionMgr();
      instance=m;
     }
    return instance;
   }

  
  private DbConnectionMgr() throws Exception
   {
    Context initContext=new InitialContext();
    Context envContext=(Context)initContext.lookup("java:/comp/env");
    //System.out.println("Looking for ["+Frontier.getDsName()+"]");
    dataSource = (DataSource)envContext.lookup(Frontier.getDsName());
    //dataSource=(DataSource)initContext.lookup(Frontier.getDsName());
    if(dataSource==null) throw new Exception("DataSource ["+Frontier.getDsName()+"] not found");
   }

  private class KeepAliveTimerTask extends java.util.TimerTask
   {
   ServletOutputStream sos=null;
   boolean shuttingDown=false;
   String threadName=null;
   int count=0;
   public KeepAliveTimerTask(String name,ServletOutputStream os)
    {
     threadName=name;
     sos = os;
    }
   public synchronized void run()
    {
      if (!shuttingDown)
       {
        Thread.currentThread().setName(threadName);
	count++;
        try {ResponseFormat.keepalive(sos);}catch(Exception e){}
        Frontier.Log("DB mgr acquire sent keepalive "+count);
	if(count>=60)
	 {
	  // give up after 5 minutes
	  Frontier.Log("DB mgr acquire keepalive giving up");
	  shuttingDown=true;
	  cancel();
	 }
       }
    }
   public synchronized void shutdown()
    {
     // set this boolean to avoid further output in case the
     // run function has already been set to go
     shuttingDown=true;
     cancel();
    }
   }

  public Connection acquire(ServletOutputStream sos) throws Exception 
   {
    Connection connection;
    Timer timer=new Timer();
    KeepAliveTimerTask timerTask=
        new KeepAliveTimerTask(Thread.currentThread().getName()+"-ka",sos);
    timer.schedule(timerTask,5000,5000);
    try
     {
      connection=dataSource.getConnection();
     }
    finally
     {
      timer.cancel();
      timerTask.shutdown();
     }
    Frontier.Log("DB mgr connection acquired");
    ResponseFormat.data_start(sos);
    return connection;
   }

 
  public void release(Connection dbConnection,ServletOutputStream sos) throws Exception 
   {
    if(dbConnection!=null) dbConnection.close();
    if (sos!=null) ResponseFormat.data_end(sos);
   }
   
   
  // This one to get frontier descriptors (or plugins), could have different DS
  public Connection getDescriptorConnection() throws Exception
   {
    Connection connection;
    connection=dataSource.getConnection();
    return connection;   
   }
 }
