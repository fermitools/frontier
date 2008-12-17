package gov.fnal.frontier;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.text.SimpleDateFormat;


public final class FrontierServlet extends HttpServlet 
 {
  private static final String frontierVersion="3.15";
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
    //  this option it returns stale data.
// Commented out because squid2.7-STABLE5 doesn't cache the negative result
//  of this when a server is down.  See squid bugzilla bug #2481.
//    String cch=request.getHeader("cache-control");
//    int idx=-1;
//    if (cch!=null)idx=cch.indexOf("max-stale");
//    if (idx>=0)
//     {
//      idx=cch.indexOf('=',idx);
//      if(idx>=0)
//       {
//	idx+=1;
//	int endidx=cch.indexOf(',',idx);
//	if(endidx==-1)endidx=cch.length();
//        response.setHeader("Cache-Control","max-age="+age+", stale-if-error="+cch.substring(idx,endidx));
//        return;
//       }
//     }
    response.setHeader("Cache-Control","max-age="+age);
   }

  private String dateHeader(long datestamp)
   {
    // could use HttpServletResponse.setDateHeader but then can't read it back
    //  for logging
    // SimpleDateFormat instances aren't thread safe so make a new one 
    //  for each use
    SimpleDateFormat df=new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z");
    df.setTimeZone(TimeZone.getTimeZone("GMT"));
    return(df.format(new Date(datestamp)));
   }

  public void service(HttpServletRequest request,HttpServletResponse response) throws ServletException,IOException
   {
    ServletOutputStream out=null;
    Frontier frontier=null;
    
    long timestamp=(new Date()).getTime();
    synchronized(mutex) 
     {
      ++count_total;
      ++count_current;
     }        
    Thread.currentThread().setName("id="+count_total);

    String globalErrorMsg="";
    try
     {
      out=response.getOutputStream();      
      response.setContentType("text/xml");
      response.setCharacterEncoding("US-ASCII");
      try
       {
        frontier=new Frontier(request);
       }
      catch(Throwable e)
       {
        Frontier.Log("Error initializing Frontier object:",e);
	setAgeExpires(request,response,Frontier.errorDefaultMaxAge());
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
        ResponseFormat.putGlobalError(out,"Error: "+throwableDescript(e));
	frontier=null;
        return; // this and all other returns here will drop down to "finally"
       }
       
      if(frontier.noCache) response.setHeader("Pragma","no-cache");
      setAgeExpires(request,response,frontier.max_age);

      long last_modified=-1;
      long if_modified_since=frontier.if_modified_since;
      try
       {
	last_modified=frontier.cachedLastModified();
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

      ResponseFormat.begin(out,frontierVersion,xmlVersion);
      ResponseFormat.transaction_start(out,frontier.payloads_num);
      
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
	    setAgeExpires(request,response,frontier.error_max_age);
	    ResponseFormat.payload_end(out,1,throwableDescript(e),"",-1,0);
	    return;
	   }

	  if(frontier.payloads_num==1)
	   {
	    if(response.isCommitted())
	     {
	      // The database was acquired with keepalives
	      if(last_modified==0)
	        Frontier.Log("response committed, too late to query for last-modified time");
	     }
	    else
	     {
	      // The database was acquired without keepalives, now set
	      //  last-modified time.   Didn't want to set it before
	      //  even if it was cached because don't want error messages
	      //  to stay cached forever.
	      if(last_modified>0)
	       {
		String lastmod=dateHeader(last_modified);
		Frontier.Log("last-modified time: "+lastmod+" (cached)");
		response.setHeader("Last-Modified",lastmod);
	       }
	      else if(last_modified==0)
	       {
		// Read the last modified time from the database.
		// Need to do it here because acquiring the database could
		//  have taken a long time and may have needed to send
		//  keepalives, and those have to be after payload_start().
		try
		 {
		  last_modified=frontier.getLastModified(out);
		  if((if_modified_since>0)&&(if_modified_since==last_modified))
		   {
		    response.setStatus(HttpServletResponse.SC_NOT_MODIFIED);
		    Frontier.Log("not modified");
		    return;
		   }
		  else
		   {
		    String lastmod=dateHeader(last_modified);
		    if(if_modified_since>0)
			Frontier.Log("modified at time: "+lastmod);
		    else
			Frontier.Log("last-modified time: "+lastmod);
		    response.setHeader("Last-Modified",lastmod);
		   }
		 }
		catch(Throwable e)
		 {
		  Frontier.Log("Error getting last-modified time:",e);
		  setAgeExpires(request,response,frontier.error_max_age);
		  ResponseFormat.payload_end(out,1,throwableDescript(e),"",-1,0);
		  return;
		 }
	       }
	     }
	   }

          try
           {
            p.send(out);
            ResponseFormat.payload_end(out,p.err_code,p.err_msg,p.md5,p.rec_num,p.full_size);
           }
          catch(Throwable e)
           {
            Frontier.Log("Error while processing payload "+i+":",e);
            ResponseFormat.payload_end(out,1,throwableDescript(e),"",-1,0);
	    if(!response.isCommitted())
	     {
	      // still have a chance to affect cache age and last-modified 
	      setAgeExpires(request,response,frontier.error_max_age);
	      // this leaves an empty header but it doesn't hurt and
	      //  there doesn't appear to be any way to delete a header
	      response.setHeader("Last-Modified","");
	     }
	    else
	     {
	      // also signal global error to tell client to clear cache
	      Frontier.Log("too late to affect header, also signaling global error");
	      globalErrorMsg=throwableDescript(e);
	     }
            break;
           }
	  p.close(out);
         }
       }
      finally
       {
        ResponseFormat.transaction_end(out);
       }
     }
    catch(Throwable e)
     {
      Frontier.Log("Internal Error: MUST NEVER HAPPEN HERE!:",e);
      // sometimes global error message gets through here to client and
      //  sometimes it causes an XML error, depending on where failure occurred
      ResponseFormat.putGlobalError(out,"Internal Error: "+throwableDescript(e));
      // no need to try to affect uncommitted headers because client should
      // re-try with flush cache, and besides this indicates coding error
     }
    finally
     {
      if(globalErrorMsg!="")
       {
	// this causes client to re-try with cache flush
        ResponseFormat.putGlobalError(out,globalErrorMsg);
       }
      if(frontier!=null)frontier.close(out);
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
     }
   }
 }
