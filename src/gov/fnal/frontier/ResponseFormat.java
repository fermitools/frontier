package gov.fnal.frontier;

import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.io.*;

public class ResponseFormat
 {
  static void handleError(String where, Exception e)
   {
    Frontier.Log("Error is ignored in ResponseFormat."+where+": ",e);
   }

  
  static String xml_str(String msg)
   {
    StringBuffer b=new StringBuffer(msg);
    while(true)
     {
      int pos=b.indexOf(">");
      if(pos<0) break;
      b.replace(pos,pos+1,"&gt;");
     }
    while(true)
     {
      int pos=b.indexOf("<");
      if(pos<0) break;
      b.replace(pos,pos+1,"&lt;");
     }
    return b.toString();
   }
   

  static void begin(ServletOutputStream out,String version,String xmlversion) throws Exception
   {
    out.print("<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\n");
    out.print("<!DOCTYPE frontier SYSTEM \"http://frontier.fnal.gov/frontier.dtd\">\n");
    out.print("<frontier version=\"");
    out.print(version);
    out.print("\" xmlversion=\"");
    out.print(xmlversion);
    out.print("\">\n");
   }

   
  static void transaction_start(ServletOutputStream out,int num) throws Exception
   {
    out.print(" <transaction payloads=\"");
    out.print(num);
    out.print("\">\n");
   }

   
  static void transaction_end(ServletOutputStream out) throws Exception
   {
    out.print(" </transaction>\n");
   }
      
   
  static void payload_start(ServletOutputStream out,String type,String version,String encoder) throws Exception
   {
    out.print("  <payload type=\"");
    out.print(type);
    out.print("\" version=\"");
    out.print(version);
    out.print("\" encoding=\"");
    out.print(encoder);
    out.print("\">\n");
    out.print("   <data>");
   }

   
  static void payload_end(ServletOutputStream out,int err_code,String err_msg,String md5,int rec_num) throws Exception
   {
    out.print("</data>\n");
    out.print("   <quality error=\"");
    out.print(err_code);
    if(err_msg.length()>0)
     {
      out.print("\" message=\"");
      out.print(xml_str(err_msg));
     }
    if(md5.length()>0)
     {
      out.print("\" md5=\"");
      out.print(md5);
     }    
    if(rec_num>=0)
     {
      out.print("\" records=\"");
      out.print(rec_num);
     }
    out.print("\"/>\n");
    out.print("  </payload>");
   }
         
  
  static void putGlobalError(ServletOutputStream out,String msg)
   {
    try
     {
      out.print("\n  <global_error msg=\""+xml_str(msg)+"\"/>\n");
     }
    catch(Exception e)
     {
      handleError("putGlobalError",e);
     }
   }

      
  static void close(ServletOutputStream out)
   {
    try
     {
      out.print("</frontier>\n");
     }
    catch(Exception e)
     {
      handleError("close",e);
     }
   }   
 }
