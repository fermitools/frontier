package gov.fnal.frontier;

import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;


public final class FrontierServlet extends HttpServlet 
 {
  private static final String frontierVersion="3.0";
  private static final String xmlVersion="1.0";
 
  public void service(HttpServletRequest request,HttpServletResponse response) throws ServletException,IOException
   {
    ServletOutputStream out=null;
    Frontier frontier=null;
    HttpSession session=null;
    
    try
     {
      writer=response.getOuputStream();      
      response.setContentType("text/xml");
      response.setCharacterEncoding("US-ASCII");
      try
       {
        session=request.getSession();
        frontier=new Frontier(request,response,session);
        response.setDateHeader("Expires",frontier.time_expire);
        if(frontier.noCache) response.setHeader("Pragma","no-cache");
       }
      catch(Exception e)
       {
        ResponseFormat.begin(out,frontierVersion,xmlVersion);
        ResponseFromat.putGlobalError(out,"Error: "+e);
        ResponseFormat.close(out);
        return;
       }
       
      ResponseFormat.begin(out,frontierVersion,xmlVersion);
      ResponseFormat.transaction_start(out,frontier.payloads_num);
      try
       {
        for(int i=0;i<frontier.payloads_num;i++)
         {
          Payload p=frontier.getPayload(i);          
          ResponseFormat.payload_start(out,p.type,p.version,p.encoder);
          try
           {
            p.send(out);                        
           }
          catch(Exception e)
           {
            Frontier.Log("Error while processing payload "+i+": "+e);
            break;
           }
          finally
           {
            ResponseFormat.payload_end(out,p.err_code,p.err_msg,p.md5,p.rec_num);
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
      ResponseFromat.putGlobalError(out,"Error: "+e);
     }
    finally
     {
      ResponseFormat.close(out);
     }
   }
 }
