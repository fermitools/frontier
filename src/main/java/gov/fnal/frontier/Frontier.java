/**
 * The main Frontier manager
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import gov.fnal.frontier.codec.*;
import jakarta.servlet.*;
import jakarta.servlet.http.*;
import java.util.*;
import java.util.regex.*;
import java.text.SimpleDateFormat;
import java.io.*;
import java.security.*;
import java.security.spec.*;


public final class Frontier
 {
  private static final SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yy HH:mm:ss.SSS z Z");    
  private static boolean initialized=false;    
  private static String conf_server_name="not_set";
  private static String conf_ds_name;
  private static String conf_file_base_dir;
  private static String conf_key_file_name;
  private static String conf_cert_file_name;
  private static final int SHORTCACHE=0;
  private static final int LONGCACHE=1;
  private static final int FOREVERCACHE=2;
  private static final int NUMCACHELENGTHS=3;
  private static String[] conf_cache_expire_hourofday = new String[NUMCACHELENGTHS];
  private static String[] conf_cache_expire_seconds = new String[NUMCACHELENGTHS];
  
  private static int verbosityLevel=0;      // Higher numbers show more messages.
                                            // In addition, from level 2, file name
					    //    and line numbers appear.
  private static boolean highVerbosity=false; // true when verbosityLevel >= 4

  public static int max_threads=100;
  public static boolean send_stale_if_error=true;
  public static int validate_last_modified_seconds=-1;
  public static String last_modified_table_name;
  public static int max_file_connections=5;
  // Note that when the DB is down, at least on SLC4 it takes about
  //  6-1/3rd minutes for DataSource.getConnection() to return
  public static int max_db_acquire_seconds=300;
  public static int max_db_execute_seconds=10;
  public static boolean expire_empty_queries_like_errors=false;
  public static byte[] cert_bytes;
  public static byte[] key_bytes;
  public static PrivateKey private_key;

  private boolean certRequested=false;

  public long max_age=-1;
  public long error_max_age=Frontier.errorDefaultMaxAge();
  public boolean noCache=false;
  public int payloads_num;
  public ArrayList<Payload> aPayloads=null;
  public long if_modified_since=-1;


  public static int getVerbosityLevel() 
   {
    return verbosityLevel;
   }

  public static boolean getHighVerbosity() 
   {
    return highVerbosity;
   }

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
  
  private static PrivateKey extractPrivateKeyFromPEM(String pem) throws Exception
   {
    // Read PEM into a private key.
    // Bouncycastle has a function for this (PEMReader), but want to avoid
    //    that dependency.

    // First convert PEM into DER, the equivalent of
    //  openssl pkcs8 -topk8 -outform DER -in key.pem -out key.der -nocrypt
    Pattern delim=Pattern.compile("-----(BEGIN|END) (RSA |)PRIVATE KEY-----\n");
    Matcher matcher=delim.matcher(pem);
    if (!matcher.find())
	throw new Exception("no recognized begin to private key");
    Boolean isRSA;
    if (matcher.group().startsWith("-----BEGIN RSA "))
	isRSA=true;
    else
	isRSA=false;
    int keystart=matcher.end();
    if (!matcher.find())
	throw new Exception("no recognized end to private key");
    String b64key=pem.substring(keystart,matcher.start()).replace("\n","");
    byte [] derdata;
    try
     {
      derdata=Base64Coder.decode(b64key.getBytes());
     }
    catch(Exception e)
     {
      throw new Exception("invalid private key");
     }

    PKCS8EncodedKeySpec keySpec;
    if (isRSA)
     {
      // these bytes were copied from openssl conversion output
      byte [] derhead={(byte)0x30,(byte)0x82,(byte)0x00,(byte)0x00,
		       (byte)0x02,(byte)0x01,(byte)0x00,(byte)0x30,
		       (byte)0x0d,(byte)0x06,(byte)0x09,(byte)0x2a, 
		       (byte)0x86,(byte)0x48,(byte)0x86,(byte)0xf7,
		       (byte)0x0d,(byte)0x01,(byte)0x01,(byte)0x01, 
		       (byte)0x05,(byte)0x00,(byte)0x04,(byte)0x82};
      byte [] pkcs8key=new byte[derhead.length+2+derdata.length];
      System.arraycopy(derhead,0,pkcs8key,0,derhead.length);
      // stuff in the 2-byte length, high byte first
      int pkcs8keylen=derdata.length;
      pkcs8key[24]=(byte)((pkcs8keylen>>8)&0xff);
      pkcs8key[25]=(byte)(pkcs8keylen&0xff);
      // length including the header (after first 4 byes) is in the header 
      pkcs8keylen+=22;
      pkcs8key[2]=(byte)((pkcs8keylen>>8)&0xff);
      pkcs8key[3]=(byte)(pkcs8keylen&0xff);
      System.arraycopy(derdata,0,pkcs8key,derhead.length+2,derdata.length);

      // Convert the DER key into a PrivateKey structure
      keySpec=new PKCS8EncodedKeySpec(pkcs8key);
     }
    else
     {
      // It's already in DER format
      keySpec=new PKCS8EncodedKeySpec(derdata);
     }
    KeyFactory kf=KeyFactory.getInstance("RSA");
    return kf.generatePrivate(keySpec);
   }
  
  protected static synchronized void init() throws Exception
   {
    if(initialized) return;

    Frontier.Log("Initializing Frontier servlet version "+FrontierServlet.frontierVersion());

    ResourceBundle prb=PropertyResourceBundle.getBundle("config");
    conf_server_name=prb.getString("ServerName");
    if(conf_server_name==null) throw new Exception("ServerName is missing in FrontierConfig");

    conf_ds_name=getPropertyString(prb,"DataSourceName");
    conf_file_base_dir=getPropertyString(prb,"FileBaseDirectory");
    if((conf_ds_name==null)&&(conf_file_base_dir==null))
      throw new Exception("Both DataSourceName and FileBaseDirectory are missing in FrontierConfig");

    if(conf_file_base_dir!=null)
     {
      String maxcons=getPropertyString(prb,"MaxFileConnections");
      if(maxcons!=null)
	max_file_connections=Integer.parseInt(maxcons);
      Frontier.Log("max file connections: "+max_file_connections);
     }

    // Verbosity level related
    String verbosity=getPropertyString(prb,"VerbosityLevel");
    if (verbosity!=null)
      verbosityLevel=Integer.parseInt(verbosity);
    highVerbosity=(verbosityLevel>=4);
    if (verbosityLevel>0)
      Frontier.Log("VerbosityLevel set to "+verbosityLevel);

    conf_key_file_name=getPropertyString(prb,"KeyFileName");
    if(conf_key_file_name!=null)
     {
      File file=new File(conf_key_file_name);
      key_bytes=new byte[(int)file.length()];
      DataInputStream dis=new DataInputStream(new FileInputStream(file));
      dis.readFully(key_bytes);
      dis.close();
      try
       {
        private_key=extractPrivateKeyFromPEM(new String(key_bytes));
       }
      catch(Exception e)
       {
        throw new Exception(conf_key_file_name+": "+e.getMessage());
       }
     }

    conf_cert_file_name=getPropertyString(prb,"CertFileName");
    if(conf_cert_file_name!=null)
     {
      File file=new File(conf_cert_file_name);
      cert_bytes=new byte[(int)file.length()];
      DataInputStream dis=new DataInputStream(new FileInputStream(file));
      dis.readFully(cert_bytes);
      dis.close();
      String cert_str=new String(cert_bytes);
      int begin=cert_str.indexOf("-----BEGIN");
      if(begin>0)
       {
	// remove excess junk before beginning of cert
        cert_bytes=cert_str.substring(begin).getBytes();
       }
     }

    String maxthreads=getPropertyString(prb,"MaxThreads");
    if(maxthreads!=null)
      max_threads=Integer.parseInt(maxthreads);
    Frontier.Log("max threads: "+max_threads);

    String sendstale=getPropertyString(prb,"SendStaleIfError");
    if(sendstale!=null)
      send_stale_if_error=Boolean.parseBoolean(sendstale);
    Frontier.Log("send stale if error: "+send_stale_if_error);

    String maxsecs=getPropertyString(prb,"MaxDbAcquireSeconds");
    if(maxsecs!=null)
     {
      max_db_acquire_seconds=Integer.parseInt(maxsecs);
      Frontier.Log("max DB acquire secs: "+max_db_acquire_seconds);
     }
    maxsecs=getPropertyString(prb,"MaxDbExecuteSeconds");
    if(maxsecs!=null)
     {
      max_db_execute_seconds=Integer.parseInt(maxsecs);
      Frontier.Log("max DB execute secs: "+max_db_execute_seconds);
     }

    String str=getPropertyString(prb,"ValidateLastModifiedSeconds");
    if(str!=null)
     {
      validate_last_modified_seconds=Integer.parseInt(str);
      Frontier.Log("validate last-modified secs: "+validate_last_modified_seconds);
      last_modified_table_name=getPropertyString(prb,"LastModifiedTableName");
      if(last_modified_table_name!=null)
        Frontier.Log("last-modified table name: "+last_modified_table_name);
     }

    str=getPropertyString(prb,"ExpireEmptyQueriesLikeErrors");
    if(str!=null)
     {
      expire_empty_queries_like_errors=Boolean.parseBoolean(str);
      Frontier.Log("expire empty queries like errors: "+expire_empty_queries_like_errors);
     }

    conf_cache_expire_hourofday[LONGCACHE]=getPropertyString(prb,"LongCacheExpireHourOfDay");
    conf_cache_expire_seconds[LONGCACHE]=getPropertyString(prb,"LongCacheExpireSeconds");
    conf_cache_expire_hourofday[SHORTCACHE]=getPropertyString(prb,"ShortCacheExpireHourOfDay");
    conf_cache_expire_seconds[SHORTCACHE]=getPropertyString(prb,"ShortCacheExpireSeconds");
    conf_cache_expire_seconds[FOREVERCACHE]=getPropertyString(prb,"ForeverCacheExpireSeconds");
    if(conf_cache_expire_seconds[FOREVERCACHE]==null)
      conf_cache_expire_seconds[FOREVERCACHE]=Integer.toString(60*60*24*365);

    // If no short time set, default it to 0 seconds
    if((conf_cache_expire_hourofday[SHORTCACHE]==null)&&
       (conf_cache_expire_seconds[SHORTCACHE]==null))
      conf_cache_expire_seconds[SHORTCACHE]="0";

    Frontier.Log("long hour: "+conf_cache_expire_hourofday[LONGCACHE]+", long secs: "+conf_cache_expire_seconds[LONGCACHE]);
    Frontier.Log("short hour: "+conf_cache_expire_hourofday[SHORTCACHE]+", short secs: "+conf_cache_expire_seconds[SHORTCACHE]);

    initialized=true;
   }


  public Frontier(HttpServletRequest req) throws Exception 
   {
    if(!initialized) init();

    DbConnectionMgr connMgr=null;
    ArrayList<Command> commandList=null;
    
    logClientDesc(req);          

    if_modified_since=req.getDateHeader("if-modified-since");
    if(if_modified_since!=-1)
      Frontier.Log("if-modified-since: "+req.getHeader("if-modified-since"));

    if(conf_cache_expire_seconds[SHORTCACHE]!=null)
     {
      //if set and less than the default error max age, use the short cache
      //  age also for errors
      long age=Long.parseLong(conf_cache_expire_seconds[SHORTCACHE]);
      if(age<error_max_age)
        error_max_age=age;
     }
    int cachelength=LONGCACHE;

    commandList=Command.parse(req);

    Command cmd=(Command)commandList.get(0);
    if(cmd.obj_name.equals("cert_request")&&cmd.obj_version.equals("1"))
     {
      certRequested=true;
      max_age=error_max_age;
      return;
     }

    payloads_num=commandList.size();
    aPayloads=new ArrayList<Payload>();
    connMgr=DbConnectionMgr.getDbConnectionMgr();
    for(int i=0;i<payloads_num;i++)
     {
      cmd=(Command)commandList.get(i);
      Payload p=new Payload(cmd,connMgr);
      if(p.noCache) noCache=true;
      String ttl=cmd.fds.getOptionalString("ttl");
      if(ttl!=null)
       {
        if(ttl.equals("short"))
	  cachelength=SHORTCACHE;
        else if(ttl.equals("forever"))
	  cachelength=FOREVERCACHE;
       }
      long p_max_age=p.time_expire/1000;
      if(max_age<0) max_age=p_max_age;
      if(p_max_age<max_age) max_age=p_max_age;
      aPayloads.add(p);
     }
    if(conf_cache_expire_seconds[cachelength]!=null)
     {
      //if set, use configured expiration seconds for requested cache length
      //  instead of Payload's
      max_age=Long.parseLong(conf_cache_expire_seconds[cachelength]);
     }
    if(conf_cache_expire_hourofday[cachelength]!=null)
     {
      Calendar cal=Calendar.getInstance();
      long now=cal.getTimeInMillis();
      //use expiration hour-of-day if it is set and sooner
      int hourofday=Integer.parseInt(conf_cache_expire_hourofday[cachelength]);
      if(hourofday<=cal.get(Calendar.HOUR_OF_DAY))
	cal.add(Calendar.DAY_OF_YEAR,1);
      cal.set(Calendar.HOUR_OF_DAY,hourofday);
      cal.set(Calendar.MINUTE,0);
      cal.set(Calendar.SECOND,0);
      long diff=(cal.getTimeInMillis()-now)/1000;
      if(diff<max_age)
	max_age=diff;
     }
    //Frontier.Log("max-age="+max_age);
   }

  public static long errorDefaultMaxAge()
   {
    return 5*60;	// errors expire in 5 minutes
   }
   
  public long cachedLastModified() throws Exception
   {
    if(highVerbosity)Frontier.Log("cachedLastModified()");
    long last_modified=0;
    // look for max last_modified of cachedLastModified() of each payload
    for(int i=0;i<payloads_num;i++)
     {
      Payload p=(Payload)aPayloads.get(i);          
      long lm=p.cachedLastModified();
      if(lm==-1)
        return -1;
      if(lm>last_modified)
        last_modified=lm;
     }
    if(highVerbosity)Frontier.Log("cachedLastModified(): last_modified: "+last_modified);
    return last_modified;
   }

  public long getLastModified(ServletOutputStream out,long if_modified_since) throws Exception
   {
    if(highVerbosity)Frontier.Log("Frontier.java:getLastModified()");
    long last_modified=0;
    for(int i=0;i<payloads_num;i++)
     {
      Payload p=(Payload)aPayloads.get(i);          
      if(highVerbosity)Frontier.Log("Frontier.java:getLastModified(): payload: "+i);
      long lm=p.getLastModified(out,if_modified_since);
      if(lm>last_modified)
        last_modified=lm;
     }
    return last_modified;
   }

  public void close(ServletOutputStream sos) throws Exception
   {
    for(int i=0;i<payloads_num;i++)
     {
      Payload p=(Payload)aPayloads.get(i);          
      p.close(sos);
     }
   }

  public boolean nonXmlResponse(HttpServletResponse response) throws Exception
   {
    if(certRequested)
     {
      if(cert_bytes==null)
        throw new Exception("no CertFileName found");
      response.setContentType("text/plain");
      response.setContentLength(cert_bytes.length);
      response.getOutputStream().write(cert_bytes);
      Frontier.Log("cert size="+cert_bytes.length);
      return true;
     }
    return false;
   }

  private void logClientDesc(HttpServletRequest req) throws Exception
   {
    String qs;
    if((qs=req.getQueryString())==null)
     {
      if((qs=req.getPathInfo())==null)
        throw new Exception("no query");
     }
    StringBuffer client_desc=new StringBuffer("");
    client_desc.append("servlet_version:");
    client_desc.append(FrontierServlet.frontierVersion());
    client_desc.append(" start threads:");
    client_desc.append(FrontierServlet.numCurrentThreads());
    client_desc.append(" query ");
    client_desc.append(qs);
    client_desc.append(" raddr ");
    client_desc.append(req.getRemoteAddr());
    client_desc.append(" frontier-id: ");
    String frontierId=req.getHeader("x-frontier-id");
    if(frontierId==null)
      client_desc.append("missing");
    else
      client_desc.append(frontierId);
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
    if(frontierId==null)
      throw new Exception("X-frontier-id header missing");
   }

   public static String getLineInfo()
    {
     return getLineInfo(2);
    }

   public static String getLineInfo(int stackIndex)
    {
     StackTraceElement ste=new Throwable().getStackTrace()[stackIndex];
     return ste.getFileName()+" +"+ste.getLineNumber();
    }

  // synchronize it to be very sure that log messages don't get 
  //  interleaved and because a SimpleDateFormat instance can't be used
  //  by multiple threads at the same time
  public static synchronized void Log(String msg)
   {
    StringBuffer buf=new StringBuffer("");
    buf.append(conf_server_name);
    buf.append(' ');
    buf.append(date_fmt.format(new Date()));
    buf.append(' ');
    String id=Thread.currentThread().getName();
    int idx=id.indexOf("id=");
    if(idx>0)
      id=id.substring(idx); // remove part before id=
    buf.append(id);
    buf.append(' ');
    if(verbosityLevel>=2)
     {
      buf.append(getLineInfo(2));
      buf.append(' ');
     }
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
  
  public static String getFileBaseDir()
   {
    return conf_file_base_dir;
   }
  
  public static String getServerName()
   {
    return conf_server_name;
   }
  
  
  public static int getMaxFileConnections()
   {
    return max_file_connections;
   }

  public static int getMaxDbAcquireSeconds()
   {
    return max_db_acquire_seconds;
   }

  public static int getMaxDbExecuteSeconds()
   {
    return max_db_execute_seconds;
   }

 } // class Frontier
