package gov.fnal.frontier;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;


public final class FrontierServlet extends HttpServlet 
 {
  private static final String frontierVersion="3.4";
  private static final String xmlVersion="1.0";
  private static int count_total=0;
  private static int count_current=0;
  private static Boolean mutex=new Boolean(true);
  
  public static String frontierVersion() {return frontierVersion;}
  public static String xmlVersion() {return xmlVersion;}
  public static int numCurrentThreads() {return count_current;}

  private static String throwableDescript(Throwable e)
   {
    return e+" at "+e.getStackTrace()[0];

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
   
 
  public void service(HttpServletRequest request,HttpServletResponse response) throws ServletException,IOException
   {
    ServletOutputStream out=null;
    Frontier frontier=null;
    
    long timestamp=(new java.util.Date()).getTime();
    synchronized(mutex) 
     {
      ++count_total;
      ++count_current;
     }        
    Thread.currentThread().setName("id="+count_total);

    try
     {
      out=response.getOutputStream();      
      response.setContentType("text/xml");
      response.setCharacterEncoding("US-ASCII");
      try
       {
        frontier=new Frontier(request,response);
        response.setDateHeader("Expires",frontier.time_expire);
        if(frontier.noCache) response.setHeader("Pragma","no-cache");
       }
      catch(Throwable e)
       {
        Frontier.Log("Error: ",e);
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
        ResponseFormat.putGlobalError(out,"Error: "+throwableDescript(e));
	out=null;
	frontier=null;
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
            p.send(out);
            ResponseFormat.payload_end(out,p.err_code,p.err_msg,p.md5,p.rec_num,p.full_size);
           }
          catch(Throwable e)
           {
            Frontier.Log("Error while processing payload "+i+": ",e);
            ResponseFormat.payload_end(out,1,throwableDescript(e),"",-1,0);
            break;
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
      Frontier.Log("Error: MUST NEVER HAPPEN HERE!: ",e);
      ResponseFormat.putGlobalError(out,"Error: "+throwableDescript(e));
     }
    finally
     {
      ResponseFormat.close(out);

      Frontier.Log("stop threads:"+count_current+" elapsed msecs="+((new java.util.Date()).getTime()-timestamp));

      synchronized (mutex) 
       {
        --count_current;
       }
     }
   }
 }
