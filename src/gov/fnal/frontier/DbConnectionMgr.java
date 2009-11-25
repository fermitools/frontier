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
import java.util.HashMap;
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
  private static final int SECONDS_BETWEEN_KEEPALIVES=5;
  private class KeepAliveTimerTask extends java.util.TimerTask
   {
    ServletOutputStream sos=null;
    boolean shuttingDown=false;
    String threadName=null;
    String waitfor=null;
    int count=0;
    int maxcount=0;
    Timer keepAliveTimer=null;

    public KeepAliveTimerTask(String name,ServletOutputStream os,String what,int seconds,Timer timer)
     {
      threadName=name;
      sos=os;
      keepAliveTimer=timer;
      setWaitFor(what,seconds);
     }

    public Timer getTimer()
     {
      return keepAliveTimer;
     }

    public synchronized void run()
     {
      if (!shuttingDown)
       {
        Thread.currentThread().setName(threadName);
	count++;
        try {ResponseFormat.keepalive(sos);}catch(Exception e){}
        Frontier.Log(waitfor+" sent keepalive "+count);
	if(count>=maxcount)
	 {
	  Frontier.Log(waitfor+" keepalive giving up");
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
      keepAliveTimer.cancel();
      cancel();
     }

    public boolean isShutdown()
     {
      return shuttingDown;
     }

    private synchronized void setWaitFor(String what,int seconds)
     {
      waitfor=what;
      count=0;
      // round count up
      maxcount=(seconds+SECONDS_BETWEEN_KEEPALIVES-1)/SECONDS_BETWEEN_KEEPALIVES;
     }
   }

  private static DbConnectionMgr instance=null;
  private static Boolean mutex=new Boolean(true);
  private DataSource dataSource=null;
  private ReentrantLock acquireLock=new ReentrantLock(true);
  // Could use a list instead of a HashMap because there will be only
  //  a small number of entries, but a hashed interface is convenient.
  // Using HashMap instead of Hashtable because it is more modern and
  //  because I prefer to make the synchronization be obvious.
  private HashMap<String,KeepAliveTimerTask> keepAliveTasks=new HashMap<String,KeepAliveTimerTask>();

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

  private synchronized void saveTimerTask(KeepAliveTimerTask task)
   {
     keepAliveTasks.put(Thread.currentThread().getName(),task);
   }

  private synchronized KeepAliveTimerTask getTimerTask()
   {
     return keepAliveTasks.get(Thread.currentThread().getName());
   }

  private synchronized void deleteTimerTask()
   {
     keepAliveTasks.remove(Thread.currentThread().getName());
   }

  public void cancelKeepAlive()
   {
    KeepAliveTimerTask task=getTimerTask();
    if(task!=null)
     {
      task.shutdown();
      deleteTimerTask();
      if(Frontier.getHighVerbosity())Frontier.Log("Cancelled KeepAlive timer task");
     }
   }

  public Connection acquire(ServletOutputStream sos) throws Exception 
   {
    Connection connection;
    Timer timer=new Timer();
    KeepAliveTimerTask task=
        new KeepAliveTimerTask(Thread.currentThread().getName()+"-ka",sos,
	    "DB acquire",Frontier.getMaxDbAcquireSeconds(),timer);
    timer.schedule(task,
    	SECONDS_BETWEEN_KEEPALIVES*1000,SECONDS_BETWEEN_KEEPALIVES*1000);
    try
     {
      Frontier.Log("Acquiring DB connection lock");
      // use a fair lock that queues tasks in order rather than using
      //  the more simple but unfair "synchronized" member function
      acquireLock.lock();
      try
       {
	// check if the corresponding KeepAliveTimerTask has already given
	//  up, and if so, don't try to get the connection because it
	//  can take a very long time if the DB server is down
        if(task.isShutdown())
          throw new Exception("Timed out waiting to acquire DB connection");
        Frontier.Log("Acquiring DB connection");
        connection=dataSource.getConnection();
       }
      finally
       {
        acquireLock.unlock();
       }
     }
    catch(Exception e)
     {
      // if error starting, shutdown keepAliveTask, otherwise leave
      //  until release() which must be guaranteed to be called
      task.shutdown();
      throw e;
     }
    Frontier.Log("DB connection acquired");
    int seconds=Frontier.getMaxDbExecuteSeconds();
    if(seconds==0)
      task.shutdown();
    else
     {
      task.setWaitFor("DB execute",seconds);
      saveTimerTask(task);
     }
    return connection;
   }

 
  public void release(Connection dbConnection,ServletOutputStream sos) throws Exception 
   {
    cancelKeepAlive();
    if(dbConnection!=null)
     {
      dbConnection.close();
      Frontier.Log("DB connection released");
     }
   }
   
   
  // This one to get frontier descriptors (or plugins), could have different DS
  public Connection getDescriptorConnection() throws Exception
   {
    Connection connection;
    connection=dataSource.getConnection();
    return connection;   
   }
 }
