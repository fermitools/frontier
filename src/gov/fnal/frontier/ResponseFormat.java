/**
 * Response formatter
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import jakarta.servlet.*;
import jakarta.servlet.http.*;
import java.util.*;
import java.io.*;

public class ResponseFormat
 {
  static void handleError(String where, Exception e)
   {
    if(!e.getClass().getSimpleName().equals("ClientAbortException"))
      Frontier.Log("Error is ignored in ResponseFormat."+where+": ",e);
   }

  
  static String xml_str(String msg)
   {
    StringBuffer b=new StringBuffer(msg);
    int oldpos=0;
    while(true)
     {
      int pos=b.indexOf("&");
      if(pos<oldpos) break;
      b.replace(pos,pos+1,"&amp;");
      oldpos=pos+1; // look beyond the new '&'
     }
    while(true)
     {
      int pos=b.indexOf("'");
      if(pos<0) break;
      b.replace(pos,pos+1,"&apos;");
     }
    while(true)
     {
      int pos=b.indexOf("\"");
      if(pos<0) break;
      b.replace(pos,pos+1,"&quot;");
     }
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

   
  static void transaction_end(ServletOutputStream out)
   {
    try
     {
      out.print(" </transaction>\n");
     }
    catch(Exception e)
     {
      handleError("transaction_end",e);
     }
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

   
  static void payload_end(ServletOutputStream out,int err_code,String err_msg,long max_age,String check,int rec_num,long full_size) throws Exception
   {
    out.print("</data>\n");
    out.print("   <quality error=\"");
    out.print(err_code);
    out.print("\" ");
    if(err_msg.length()>0)
     {
      out.print("message=\"");
      err_msg=Frontier.getServerName()+" "+err_msg;
      out.print(xml_str(err_msg));
      out.print("\" ");
     }
    if(max_age>0)
     {
      out.print("max_age=\"");
      out.print(max_age);
      out.print("\" ");
     }
    if(check.length()>0)
     {
      out.print(check);
      out.print(" ");
     }
    if(rec_num>=0)
     {
      out.print("records=\"");
      out.print(rec_num);
      out.print("\" ");
     }
    out.print("full_size=\"");
    out.print(full_size);
    out.print("\"/>\n");
    out.print("  </payload>\n");
   }
         
  static void commit(ServletOutputStream out) throws Exception
   {
    // a flush finishes & sends the response header and everything else
    //  written so far
    out.flush();
   }

  static void keepalive(ServletOutputStream out) throws Exception
   {
    out.print("\n   <keepalive />");
    commit(out); /* make sure keepalive is sent immediately */
   }
  
  static void putGlobalError(ServletOutputStream out,String msg)
   {
    msg=Frontier.getServerName()+" "+msg;
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
