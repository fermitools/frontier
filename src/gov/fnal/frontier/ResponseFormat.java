package gov.fnal.frontier;

import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;
import java.io.*;

public class ResponseFormat
 {
  static void handleError(String where, Exception e)
   {
    System.out.ptrintln("Error is ignored in ResponseFormat."+where+":"+e);
   }


  static void begin(ServletOutputStream out,String version,String xmlversion) throws Exception
   {
    out.write("<?xml version=\"1.0\" encoding=\"US-ASCII\"?>\n");
    out.write("<!DOCTYPE frontier SYSTEM \"http://frontier.fnal.gov/frontier.dtd\">\n");
    out.write("<frontier version=\"");
    out.write(version);
    out.write("\" xmlversion=\"");
    out.write(xmlversion);
    out.write("\">\n");
   }

   
  static void transaction_start(ServletOutputStream out,int num) throws Exception
   {
    out.write(" <transaction payloads=\"");
    out.write(num);
    out.write("\">\n");
   }

   
  static void transaction_end(ServletOutputStream out) throws Exception
   {
    out.write(" </transaction>\n");
   }
      
   
  static void payload_start(ServletOutputStream out,String type,String version,String encoder) throws Exception
   {
    out.write("  <payload type=\"");
    out.write(type);
    out.write("\" version=\"");
    out.write(version);
    out.write("\" encoding=\"");
    out.write(encoding);
    out.write("\">\n");
    out.write("   <data>");
   }

   
  static void payload_end(ServletOutputStream out,int err_code,String err_msg,String md5,int rec_num) throws Exception
   {
    out.write("</data>\n");
    out.write("   <quality error=\"");
    out.write(err_code);
    if(err_msg.length()>0)
     {
      out.write("\" message=\"");
      out.write(java.net.URLEncoder.encode(err_msg,"US_ASCII"));
     }
    if(md5.length()>0)
     {
      out.write("\" md5=\"");
      out.write(md5);
     }    
    if(rec_num>=0)
     {
      out.write("\" records=\"");
      out.write(rec_num);
     }
    out.write("\"/>\n");
    out.write("  </payload>");
   }
         
  
  static void putGlobalError(ServletOutputStream out,String msg)
   {
    try
     {
      out.write("\n  <global_error>"+java.net.URLEncoder.encode(msg,"US_ASCII")+"</global_error>\n");
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
      out.write("</frontier>\n");
     }
    catch(Exception e)
     {
      handleError("close",e);
     }
   }   
 }
