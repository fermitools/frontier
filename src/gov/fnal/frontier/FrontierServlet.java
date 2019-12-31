/**
 * Frontier http servlet interface
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.text.SimpleDateFormat;
import com.jcraft.jzlib.*;


public final class FrontierServlet extends HttpServlet 
 {
  private static final String frontierVersion="3.40";
  private static final String xmlVersion="1.0";
  private static int count_total=0;
  private static int count_current=0;
  private static Boolean mutex=new Boolean(true);
  
  public static String frontierVersion() {return frontierVersion;}
  public static String xmlVersion() {return xmlVersion;}
  public static int numCurrentThreads() {return count_current;}

  private static String throwableDescript(Throwable e)
   {
    String msg=e.toString().trim();
    StackTraceElement[] stacktrace=e.getStackTrace();
    if(stacktrace.length>0)
      msg+=" at "+stacktrace[0];
    if(msg.startsWith("java.lang."))
      msg=msg.substring(10);
    return msg;
   }
  
  public void init()
   {
    Thread.currentThread().setName("FrontierInit");
    try
     {
      Frontier.init();
     }
    catch(Throwable e)
     {
      Frontier.Log("Frontier.init() failed:",e);
     }
   }
   
  private void setAgeExpires(HttpServletRequest request,HttpServletResponse response,long age)
   {
    // If max-stale is set by client, respond with corresponding 
    //  stale-if-error.  With squid2.7, this causes an error to
    //  be returned when the origin server can't be contacted and
    //  an old copy exists in the cache.  Before squid2.7 or without
    //  this option it returns stale data.  squid2.7 caches this
    //  negative error for a configured period of time so the server
    //  won't be hit too hard for requests.
    String cch=request.getHeader("cache-control");
    int idx=-1;
    if(cch!=null)idx=cch.indexOf("max-stale");
    if(Frontier.send_stale_if_error&&(idx>=0))
     {
      idx=cch.indexOf('=',idx);
      if(idx>=0)
       {
        idx+=1;
        int endidx=cch.indexOf(',',idx);
        if(endidx==-1)endidx=cch.length();
        response.setHeader("Cache-Control","max-age="+age+", stale-if-error="+cch.substring(idx,endidx));
        return;
       }
     }
    else
     response.setHeader("Cache-Control","max-age="+age);
   }

  public static String dateHeader(long datestamp)
   {
    // could use HttpServletResponse.setDateHeader but then can't read it back
    //  for logging
    // SimpleDateFormat instances aren't thread safe so make a new one 
    //  for each use
    SimpleDateFormat df=new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z");
    df.setTimeZone(TimeZone.getTimeZone("GMT"));
    return(df.format(new Date(datestamp)));
   }

  public static long parseDateHeader(String dateheader) throws Exception
   {
    SimpleDateFormat df=new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z");
    return df.parse(dateheader).getTime();
   }

  private String error_payload_end(ServletOutputStream out,HttpServletRequest request,HttpServletResponse response,String descript,String check,long error_max_age) throws Exception
   {
    String msg="";
    long max_age=0;
    if(response.isCommitted())
     {
      // Response is commited, too late to affect http header.
      // Signal global error to tell clients to clear cache.
      // This is needed if a Last-Modified header was already sent because
      //  a normal cache check will think the cache is up to date if
      //  the modification time hasn't changed.  It's also needed for
      //  older clients that don't honor max_age in the payload; older
      //  clients would only re-try if the global error was signaled.
      Frontier.Log("too late to affect header, signaling global error");
      msg=descript;
      // set max cache age in the payload
      max_age=error_max_age;
     }
    else
     {
      // tell proxies to cache the error for a short time
      setAgeExpires(request,response,error_max_age);
      // There doesn't appear to be any way to delete a header, so set
      //  the Last-Modified time to the oldest possible time, so any
      //  newer response will override it. Both squid and this servlet
      //  treat a zero datestamp as if Last-Modified never matches, so
      //  even if an error condition persists but the message changes,
      //  the message will get updated.
      response.setHeader("Last-Modified",dateHeader(0));
     }
    ResponseFormat.payload_end(out,1,descript,max_age,check,-1,0);
    return msg;
  }

  public void service(HttpServletRequest request,HttpServletResponse response) throws ServletException,IOException
   {
    if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service()");
    ServletOutputStream out=null;
    Frontier frontier=null;
    
    long timestamp=(new Date()).getTime();
    int newid;
    synchronized(mutex) 
     {
      newid=++count_total;
      ++count_current;
     }        
    Thread.currentThread().setName(Frontier.getServerName()+" id="+newid);

    String globalErrorMsg="";
    Throwable fatalErrorThrowable=null;
    boolean nonXml=false;
    try
     {
      out=response.getOutputStream();      
      response.setContentType("text/xml");
      response.setCharacterEncoding("US-ASCII");
      try
       {
        if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): before Frontier(request)");
        frontier=new Frontier(request);
        if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): after Frontier(request)");
       }
      catch(Throwable e)
       {
        Frontier.Log("Error initializing Frontier object:",e);
	setAgeExpires(request,response,Frontier.errorDefaultMaxAge());
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
	globalErrorMsg="Error: "+throwableDescript(e);
	frontier=null;
        return; // this and all other returns here will drop down to "finally"
       }
       
      if(frontier.noCache){ 
        if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): frontier.noCache");
        response.setHeader("Pragma","no-cache");
      }
      setAgeExpires(request,response,frontier.max_age);
      if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): setAgeExpires: frontier.max_age: "+ frontier.max_age);

      try
       {
	if(frontier.nonXmlResponse(response))
	 {
	  nonXml=true;
	  return;
	 }
       }
      catch(Throwable e)
       {
        Frontier.Log("Error in nonXmlResponse: "+throwableDescript(e));
	return;
       }

      long last_modified=-1;
      long if_modified_since=frontier.if_modified_since;
      try
       {
	last_modified=frontier.cachedLastModified();
        if(Frontier.getHighVerbosity())Frontier.Log("last_modified: "+last_modified+": "+dateHeader(last_modified)+" if_modified_since: "+if_modified_since+": "+dateHeader(if_modified_since));
	if((if_modified_since>0)&&(if_modified_since==last_modified))
         {
          response.setStatus(HttpServletResponse.SC_NOT_MODIFIED);
	  //no need to log the last_modified time, already been logged
	  // with the if_modified_since
	  Frontier.Log("not modified (cached)");
	  return;
         }
       }
      catch(Throwable e)
       {
        Frontier.Log("Error getting cached last-modified time:",e);
        setAgeExpires(request,response,frontier.error_max_age);
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
	globalErrorMsg=throwableDescript(e);
        return;
       }

      if(count_current>frontier.max_threads)
       {
        // too many threads in use by this servlet, don't acquire database
        response.setStatus(HttpServletResponse.SC_SERVICE_UNAVAILABLE);
        setAgeExpires(request,response,frontier.error_max_age);
	Frontier.Log("rejecting because servlet too busy");
	return;
       }

      if(last_modified>0)
       {
	String lastmod=dateHeader(last_modified);
	if(if_modified_since>0)
	  Frontier.Log("modified at time: "+lastmod+" (cached)");
	else
	  Frontier.Log("last-modified time: "+lastmod+" (cached)");
	response.setHeader("Last-Modified",lastmod);
       }

      ResponseFormat.begin(out,frontierVersion,xmlVersion);
      ResponseFormat.transaction_start(out,frontier.payloads_num);
      
      if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): before per payload loop");
      try
       {
        for(int i=0;i<frontier.payloads_num;i++)
         {
          Payload p=(Payload)frontier.aPayloads.get(i);          
          ResponseFormat.payload_start(out,p.type,p.version,p.encoder);
	  try
	   {
	    // acquires database if hasn't already been acquired
	    p.start(out);
	   }
	  catch(Throwable e)
	   {
	    Frontier.Log("Error acquiring database:",e);
	    globalErrorMsg=error_payload_end(out,request,response,throwableDescript(e),p.getCheck(),frontier.error_max_age);
	    return;
	   }

	  // if reach this point, p.close must be called to release database
	  //  and cancel keepalive timer
	  try
	   {
	    if((frontier.payloads_num==1)&&(last_modified==0))
	     {
	      if(response.isCommitted())
	       {
		// The database was acquired with keepalives
		Frontier.Log("response committed, too late to query for last-modified time");
	       }
	      else
	       {
		// The database was acquired without keepalives, read
		//  last-modified time from the database. 
		// Need to do it here because acquiring the database could
		//  have taken a long time and may have needed to send
		//  keepalives, and those have to be after payload_start().
		try
		 {
		  if(Frontier.getHighVerbosity())Frontier.Log("FrontierServlet.java:service(): Going to call frontier.getLastModified()");
		  last_modified=frontier.getLastModified(out,if_modified_since);
		 }
		catch(Throwable e)
		 {
		  Frontier.Log("Error getting last-modified time: "+throwableDescript(e));
		  globalErrorMsg=error_payload_end(out,request,response,throwableDescript(e),p.getCheck(),frontier.error_max_age);
		  return;
		 }
		if(response.isCommitted())
		 {
		  Frontier.Log("response committed while querying last-modified time, too late to use");
		 }
		else if((if_modified_since>0)&&(if_modified_since==last_modified))
		 {
		  response.setStatus(HttpServletResponse.SC_NOT_MODIFIED);
		  Frontier.Log("not modified");
		  return;
		 }
		else if(last_modified>0)
		 {
		  String lastmod=dateHeader(last_modified);
		  if(if_modified_since>0)
		      Frontier.Log("modified at time: "+lastmod);
		  else
		      Frontier.Log("last-modified time: "+lastmod);
		  response.setHeader("Last-Modified",lastmod);
		 }
	       }
	     }

	    try
	     {
	      p.send(out);

	      long max_age=0;
	      if((p.rec_num==0)&&frontier.expire_empty_queries_like_errors)
	       {
		if(response.isCommitted())
		 {
		  // Empty responses can be an error as well, so make sure
		  //  they're cached only a short time by telling the client
		  //  to limit the age.
	          max_age=frontier.error_max_age;
		  Frontier.Log("empty response, setting payload max age to "+max_age);
		 }
		else
		 {
		  if(last_modified>0)
		   {
		    // Also tell the client to limit the age, because
		    //   otherwise a later query with an unchanged
		    //   If-Modified-Since time would not get the short
		    //   error_max_age, it would get the regular long max_age.
		    //   This was noticed because the increase in the
		    //   Cache-control max-age triggered a bug in squid-3.5.21
		    //   which caused downstream squids to pass all queries
		    //   through to an upstream squid.  There's no way in
		    //   HttpServletResponse to remove a Last-Modified header
		    //   completely; it can be blanked, which squid seems to
		    //   be able to handle, but it breaks the standard so
		    //   this alternative is used instead.
	            max_age=frontier.error_max_age;
		   }
		  setAgeExpires(request,response,frontier.error_max_age);
		  Frontier.Log("empty response, setting max age and payload max age to "+
				frontier.error_max_age);
		 }
	       }

	      ResponseFormat.payload_end(out,p.err_code,p.err_msg,max_age,p.getCheck(),p.rec_num,p.full_size);
	     }
	    catch(Throwable e)
	     {
	      if(e.getClass().getSimpleName().equals("ClientAbortException"))
	       {
		// client has disconnected, don't send anything else
		Frontier.Log("Client disconnected while processing payload "+i+": "+throwableDescript(e));
	       }
	      else
	       {
		Frontier.Log("Error while processing payload "+i+":",e);
		globalErrorMsg=error_payload_end(out,request,response,throwableDescript(e),p.getCheck(),frontier.error_max_age);
		break;
	       }
	     }
	   }
	  finally
	   {
	    try 
	     {
	      p.close(out);
	     }
	    catch(Exception e)
	     {
	      Frontier.Log("Error closing payload: "+throwableDescript(e));
	      fatalErrorThrowable=e;
	     }
	   }
         }
       }
      finally
       {
	ResponseFormat.transaction_end(out);
       }
     }
    catch(Throwable e)
     {
      if(fatalErrorThrowable==null)
       {
	Frontier.Log("Internal Error: MUST NEVER HAPPEN HERE!: "+throwableDescript(e));
	fatalErrorThrowable=e;
       }
      else
	Frontier.Log("Internal Error: MUST NEVER HAPPEN HERE!:",e);

      // sometimes global error message gets through here to client and
      //  sometimes it causes an XML error, depending on where failure occurred
      if(globalErrorMsg.equals(""))
        globalErrorMsg="Internal Error: "+throwableDescript(e);

      // no need to try to affect uncommitted headers because client should
      // re-try with flush cache, and besides this indicates coding error
     }
    finally
     {
      if(!globalErrorMsg.equals(""))
       {
	// this causes client to re-try with cache flush
        ResponseFormat.putGlobalError(out,globalErrorMsg);
       }
      if(frontier!=null)
       {
	try
	 {
	  frontier.close(out);
	 }
	catch(Exception e)
	 {
	  Frontier.Log("Error closing connection: "+throwableDescript(e));
	  if(fatalErrorThrowable==null)
	    fatalErrorThrowable=e;
	 }
       }
      if(!nonXml)
        ResponseFormat.close(out);

      if(response.isCommitted())
       {
	// This means that there will be no Content-Length in the header
	//   and that the connection will be dropped to indicate the end
	//   of the response.
	// It happens any time the response buffer (default 8k) is filled up
	//  or ResponseFormat.keepalive() is called.
        Frontier.Log("response was precommitted");
       }

      Frontier.Log("stop threads="+count_current+" msecs="+((new Date()).getTime()-timestamp));

      synchronized (mutex) 
       {
        --count_current;
       }

      if(fatalErrorThrowable!=null)
       {
        String touchpath=getServletConfig().getServletContext().getRealPath("WEB-INF/classes/config.properties");
	Frontier.Log("FATAL ERROR:",fatalErrorThrowable);
	Frontier.Log("Touching "+touchpath+" to force restart of servlet");
	try 
	 {
	  File file=new File(touchpath);
	  if(!file.setLastModified(Calendar.getInstance().getTimeInMillis()))
	    Frontier.Log("Touching file failed");
	 }
	catch(Exception e)
	 {
	  Frontier.Log("Ignoring error with touch:",e);
	 }
       }

      if(!globalErrorMsg.equals("") )
       {
	String opts=request.getHeader("X-Frontier-Opts");
	if((opts!=null)&&(opts.contains("DontCacheErrors")))
	 {
	  // Throwing an exception prevents sending the last (zero length)
	  //  chunk of HTTP/1.1 transfer encoding, which prevents proxies
	  //  from caching the response.  This is needed for proxies that
	  //  ignore client cache flush requests (such as Cloudflare).
	  Frontier.Log("closing immediately");
	  throw new ServletException("close now");
	 }
       }
     }
   }
 }
