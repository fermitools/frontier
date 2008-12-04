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
import java.util.concurrent.locks.ReentrantLock;
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
  private ReentrantLock acquireLock=new ReentrantLock(true);

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
        Frontier.Log("DB acquire sent keepalive "+count);
	if(count>=60)
	 {
	  // Give up after 5 minutes
	  // Note that when the DB is down, at least on SLC4 it takes about
	  //  6-1/3rd minutes for dataSource.getConnection() to return
	  Frontier.Log("DB acquire keepalive giving up");
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
    public boolean isShutdown()
     {
      return shuttingDown;
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
      Frontier.Log("Acquiring DB connection lock");
      // use a fair lock that queues tasks in order rather than using
      //  the more simple but unfair "synchronized" member function
      acquireLock.lock();
      try
       {
	// check if the corresponding timerTask has already given up,
	//  and if so, don't try to get the connection because it
	//  can take a very long time of the DB server is down
        if(timerTask.isShutdown())
          throw new Exception("Timed out waiting to acquire DB connection");
        Frontier.Log("Acquiring DB connection");
        connection=dataSource.getConnection();
       }
      finally
       {
        acquireLock.unlock();
       }
     }
    finally
     {
      timer.cancel();
      timerTask.shutdown();
     }
    Frontier.Log("DB connection acquired");
    return connection;
   }

 
  public void release(Connection dbConnection,ServletOutputStream sos) throws Exception 
   {
    if(dbConnection!=null) dbConnection.close();
   }
   
   
  // This one to get frontier descriptors (or plugins), could have different DS
  public Connection getDescriptorConnection() throws Exception
   {
    Connection connection;
    connection=dataSource.getConnection();
    return connection;   
   }
 }
