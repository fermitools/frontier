package gov.fnal.frontier;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;


public final class FrontierServlet extends HttpServlet 
 {
  private static final String frontierVersion="3.1";
  private static final String xmlVersion="1.0";
  
  
  public void init()
   {
    try
     {
      Frontier.init();
     }
    catch(Exception e)
     {
      Frontier.Log("Frontier.init() failes:",e);
     }
   }
   
 
  public void service(HttpServletRequest request,HttpServletResponse response) throws ServletException,IOException
   {
    ServletOutputStream out=null;
    Frontier frontier=null;
    
    try
     {
      out=response.getOutputStream();      
      response.setContentType("text/xml");
      response.setCharacterEncoding("US-ASCII");
      try
       {
        frontier=new Frontier(request,response);
        response.setDateHeader("Expires",java.lang.System.currentTimeMillis()+frontier.time_expire);
        if(frontier.noCache) response.setHeader("Pragma","no-cache");
       }
      catch(Exception e)
       {
        Frontier.Log("Error: ",e);
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
        ResponseFormat.putGlobalError(out,"Error: "+e);
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
          catch(Exception e)
           {
            Frontier.Log("Error while processing payload "+i+": ",e);
            ResponseFormat.payload_end(out,1,""+e,"",-1,0);
            break;
           }
         }
       }
      finally
       {
        ResponseFormat.transaction_end(out);
       }
     }
    catch(Exception e)
     {
      Frontier.Log("Error: MUST NEVER HAPPEN HERE!: ",e);
      ResponseFormat.putGlobalError(out,"Error: "+e);
     }
    finally
     {
      ResponseFormat.close(out);
     }
   }
 }
