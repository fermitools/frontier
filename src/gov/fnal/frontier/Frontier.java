package gov.fnal.frontier;

import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.io.*;


public final class Frontier
 {
  private static int count_total=0;
  private static int count_current=0;
  private static Boolean mutex=new Boolean(true);
  private static final SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yy HH:mm:ss.SSS z Z");    
  private static boolean initialized=false;    
  private static String conf_server_name="not_set";
  private static String conf_ds_name;
  private static String conf_xsd_table;
  
  private static String conf_monitor_node;  // MonAlisa stuff, node address events to be sent to
  private static String conf_monitor_delay; // MonAlisa stuff, delay between events sent,in msec
  private static Monitor monitor;           // MonAlisa stuff, wrapper class
  
    
  public long time_expire=-1;
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
        
    initialized=true;
   }
          

  public Frontier(HttpServletRequest req,HttpServletResponse res) throws Exception 
   {
    if(!initialized) init();

    int local_id;
    long timestamp=(new java.util.Date()).getTime();
    DbConnectionMgr connMgr=null;
    ArrayList commandList=null;
    
    synchronized(mutex) 
     {
      ++count_total;
      ++count_current;
      local_id=count_total;
     }        
    Thread.currentThread().setName("id="+local_id);
    if(monitor!=null) monitor.increment();

    try 
     {
      logClientDesc(req);          
      connMgr=DbConnectionMgr.getDbConnectionMgr();
      commandList=Command.parse(req);
      payloads_num=commandList.size();
      aPayloads=new ArrayList();
      for(int i=0;i<payloads_num;i++)
       {
        Command cmd=(Command)commandList.get(i);
        Payload p=new Payload(cmd,connMgr);
        if(p.noCache) noCache=true;
        if(time_expire<0) time_expire=p.time_expire;
        if(p.time_expire<time_expire) time_expire=p.time_expire;
        aPayloads.add(p);
       }
     } 
    finally 
     {
      synchronized (mutex) 
       {
        --count_current;
       }
      Log("stop threads:"+count_current+" ET="+((new java.util.Date()).getTime()-timestamp));
     }
   }

   
  private void logClientDesc(HttpServletRequest req) throws Exception
   {
    String queryString=java.net.URLDecoder.decode(req.getQueryString(),"US-ASCII"); StringBuffer client_desc=new StringBuffer("");
    client_desc.append("start threads:");
    client_desc.append(count_current);
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

     
  public static void Log(String msg,Exception e)
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
 } // class Frontier
