/**
 * Database connection manager
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

import java.util.Timer;
import java.util.Date;
import java.util.HashMap;
import java.util.concurrent.locks.ReentrantLock;
import jakarta.servlet.ServletOutputStream;

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
  private static Boolean mutex=Boolean.TRUE;
  private ReentrantLock acquireLock=new ReentrantLock(true);
  private Boolean counterMutex=Boolean.TRUE;
  private int numAcquiredConnections=0;
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

  public void acquire(FrontierPlugin plugin,ServletOutputStream sos) throws Exception 
   {
    Timer timer=new Timer();
    KeepAliveTimerTask task=
        new KeepAliveTimerTask(Thread.currentThread().getName()+"-ka",sos,
	    "DB acquire",Frontier.getMaxDbAcquireSeconds(),timer);
    timer.schedule(task,
    	SECONDS_BETWEEN_KEEPALIVES*1000,SECONDS_BETWEEN_KEEPALIVES*1000);
    long timestamp=(new Date()).getTime();
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
        plugin.fp_acquire();
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
    int active;
    synchronized(counterMutex)
     {
      active=++numAcquiredConnections;
     }
    Frontier.Log("DB connection acquired active="+active+" msecs="+((new Date()).getTime()-timestamp));
    int seconds=Frontier.getMaxDbExecuteSeconds();
    if(seconds==0)
      task.shutdown();
    else
     {
      task.setWaitFor("DB execute",seconds);
      saveTimerTask(task);
     }
   }

 
  public void release(FrontierPlugin plugin,ServletOutputStream sos) throws Exception 
   {
    cancelKeepAlive();
    int remaining;
    synchronized(counterMutex)
     {
      remaining=--numAcquiredConnections;
     }
    Frontier.Log("DB connection released remaining="+remaining);
    plugin.fp_release();
   }

 }
