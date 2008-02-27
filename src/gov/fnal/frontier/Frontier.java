package gov.fnal.frontier;

import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.io.*;


public final class Frontier
 {
  private static final SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yy HH:mm:ss.SSS z Z");    
  private static boolean initialized=false;    
  private static String conf_server_name="not_set";
  private static String conf_ds_name;
  private static String conf_xsd_table;
  private static final int SHORTCACHE=0;
  private static final int LONGCACHE=1;
  private static final int NUMCACHELENGTHS=2;
  private static String[] conf_cache_expire_hourofday = new String[NUMCACHELENGTHS];
  private static String[] conf_cache_expire_seconds = new String[NUMCACHELENGTHS];
  
  private static String conf_monitor_node;  // MonAlisa stuff, node address events to be sent to
  private static String conf_monitor_delay; // MonAlisa stuff, delay between events sent,in msec
  private static Monitor monitor;           // MonAlisa stuff, wrapper class
  
  private static boolean use_fdo_cache;     // If true, the FDO info is created once (
                                            // based on XSD or else)
  
    
  public long time_expire=-1;
  public long error_expire=0;
  public boolean noCache=false;
  public int payloads_num;
  public ArrayList aPayloads=null;
  
  
  
  private static String getPropertyString(ResourceBundle rb,String name) throws Exception
   {
    try
     {
      String p=rb.getString(name);
      return p;
     }
    catch(MissingResourceException e)
     {          
      return null;
     }
   }
  
  
  protected static synchronized void init() throws Exception
   {
    if(initialized) return;
    ResourceBundle prb=PropertyResourceBundle.getBundle("config");
    conf_server_name=prb.getString("ServerName");
    conf_ds_name=prb.getString("DataSourceName");
    conf_xsd_table=prb.getString("XsdTableName");
        
    if(conf_server_name==null) throw new Exception("ServerName is missing in FrontierConfig");
    if(conf_ds_name==null) throw new Exception("DataSourceName is missing in FrontierConfig");
    if(conf_xsd_table==null) throw new Exception("XsdTableName is missing in FrontierConfig");

    conf_cache_expire_hourofday[LONGCACHE]=getPropertyString(prb,"LongCacheExpireHourOfDay");
    conf_cache_expire_seconds[LONGCACHE]=getPropertyString(prb,"LongCacheExpireSeconds");
    conf_cache_expire_hourofday[SHORTCACHE]=getPropertyString(prb,"ShortCacheExpireHourOfDay");
    conf_cache_expire_seconds[SHORTCACHE]=getPropertyString(prb,"ShortCacheExpireSeconds");

    // If no short time set, default it to 0 seconds
    if((conf_cache_expire_hourofday[SHORTCACHE]==null)&&
       (conf_cache_expire_seconds[SHORTCACHE]==null))
      conf_cache_expire_seconds[SHORTCACHE]="0";

    Frontier.Log("long hour: "+conf_cache_expire_hourofday[LONGCACHE]+", long secs: "+conf_cache_expire_seconds[LONGCACHE]);
    Frontier.Log("short hour: "+conf_cache_expire_hourofday[SHORTCACHE]+", short secs: "+conf_cache_expire_seconds[SHORTCACHE]);

    conf_monitor_node=getPropertyString(prb,"MonitorNode");
    conf_monitor_delay=getPropertyString(prb,"MonitorMillisDelay");
    if(conf_monitor_node!=null && conf_monitor_delay!=null) 
     {
      try
       {
        monitor=new Monitor(conf_monitor_node,conf_monitor_delay);
        monitor.setDaemon(true);
        monitor.start();
       }
      catch(Exception e)
       {
        Frontier.Log("MonAlisa monitor failed to start:",e);
        monitor=null;
       }
     } 
    else Frontier.Log("MonAlisa monitor was not configured.");
    
    String tmp=getPropertyString(prb,"UseFdoCache");
    use_fdo_cache=(tmp!=null && tmp.equalsIgnoreCase("yes"));
    if(use_fdo_cache) Frontier.Log("FDO cache is in use");
    else Frontier.Log("FDO cache is DISABLED");
        
    initialized=true;
   }
          

  public Frontier(HttpServletRequest req,HttpServletResponse res) throws Exception 
   {
    if(!initialized) init();

    DbConnectionMgr connMgr=null;
    ArrayList commandList=null;
    
    if(monitor!=null) monitor.increment();

    logClientDesc(req);          
    connMgr=DbConnectionMgr.getDbConnectionMgr();
    commandList=Command.parse(req);
    payloads_num=commandList.size();
    aPayloads=new ArrayList();
    int cachelength=LONGCACHE;
    for(int i=0;i<payloads_num;i++)
     {
      Command cmd=(Command)commandList.get(i);
      Payload p=new Payload(cmd,connMgr);
      if(p.noCache) noCache=true;
      String ttl=cmd.fds.getOptionalString("ttl");
      if((ttl!=null)&&(ttl.equals("short"))) cachelength=SHORTCACHE;
      if(time_expire<0) time_expire=p.time_expire;
      if(p.time_expire<time_expire) time_expire=p.time_expire;
      aPayloads.add(p);
     }
    if(conf_cache_expire_seconds[cachelength]!=null)
     {
      //if set, use configured expiration seconds for requested cache length
      //  instead of Payload's milliseconds
      time_expire=Long.parseLong(conf_cache_expire_seconds[cachelength])*1000;
     }
    Calendar cal=Calendar.getInstance();
    long now=cal.getTimeInMillis();
    if(conf_cache_expire_hourofday[cachelength]!=null)
     {
      //use expiration hour-of-day if it is set and sooner
      int hourofday=Integer.parseInt(conf_cache_expire_hourofday[cachelength]);
      if(hourofday<=cal.get(Calendar.HOUR_OF_DAY))
	cal.add(Calendar.DAY_OF_YEAR,1);
      cal.set(Calendar.HOUR_OF_DAY,hourofday);
      cal.set(Calendar.MINUTE,0);
      cal.set(Calendar.SECOND,0);
      long diff=cal.getTimeInMillis()-now;
      if(diff<time_expire)
	time_expire=diff;
     }
    Frontier.Log("seconds to expiration="+(time_expire/1000));
    time_expire+=now;
    error_expire=now+5*60*1000;		// errors expire in 5 minutes
   }

   
  private void logClientDesc(HttpServletRequest req) throws Exception
   {
    String queryString=java.net.URLDecoder.decode(req.getQueryString(),"US-ASCII"); 
    StringBuffer client_desc=new StringBuffer("");
    client_desc.append("servlet_version:");
    client_desc.append(FrontierServlet.frontierVersion());
    client_desc.append(" start threads:");
    client_desc.append(FrontierServlet.numCurrentThreads());
    client_desc.append(" query ");
    client_desc.append(queryString);
    client_desc.append(" raddr ");
    client_desc.append(req.getRemoteAddr());
    client_desc.append(" frontier-id: ");
    client_desc.append(req.getHeader("x-frontier-id"));
    for(Enumeration en=req.getHeaderNames();en.hasMoreElements();)
     {
      String name=(String)en.nextElement();
      if(name.compareToIgnoreCase("via")==0 || name.compareToIgnoreCase("x-forwarded-for")==0)
       {
        client_desc.append(' ');
        client_desc.append(name);
        client_desc.append(": ");
        client_desc.append(req.getHeader(name));
       }
     }            
    Log(client_desc.toString());
   }
                
     
  public static void Log(String msg)
   {
    StringBuffer buf=new StringBuffer("");
    buf.append(conf_server_name);
    buf.append(' ');
    buf.append(date_fmt.format(new java.util.Date()));
    buf.append(' ');
    buf.append(Thread.currentThread().getName());
    buf.append(' ');
    buf.append(msg);
    System.out.println(buf.toString());
   }

     
  public static void Log(String msg,Throwable e)
   {     
    ByteArrayOutputStream baos=new ByteArrayOutputStream();
    PrintWriter pw=new PrintWriter(baos);
    e.printStackTrace(pw);
    pw.flush();
    Log(msg+"\n"+baos.toString());
   }
     
    
  public static String getDsName()
   {
    return conf_ds_name;
   }
  
  
  public static String getXsdTableName()
   {
    return conf_xsd_table;
   }
   
  public static boolean isFdoCache()
   {
    return use_fdo_cache;
   }
 } // class Frontier
