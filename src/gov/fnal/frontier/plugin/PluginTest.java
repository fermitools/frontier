/**
 * A test for the frontier plugin system
 * $Id$
 * 
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier.plugin;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.*;
import java.sql.*;

public class PluginTest implements FrontierPlugin
 {
  public String[] fp_getMethods()
   {
    String[] ret=new String[1];
    ret[0]="DEFAULT";
    return ret;
   }
   
   
  public MethodDesc fp_getMethodDesc(String name) throws Exception
   {
    MethodDesc ret=new MethodDesc("DEFAULT","get",false,((long)60*60*24*7*1000),"free","public");
    return ret;
   }
   
   
  public int fp_get(java.sql.Connection con,DbConnectionMgr mgr,Encoder enc,String method) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);
    PreparedStatement stmt=null;
    ResultSet rs=null;
    int row_count=0;
    try
     {
      stmt=con.prepareStatement("select to_char(SYSDATE) as datetime from dual");
      rs=stmt.executeQuery();
      rs.next();
      String s=rs.getString(1);
      enc.writeString(s);
      enc.writeEOR();
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
     }        
    return row_count;
   }
   
  public int fp_meta(Encoder enc,String method) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);
    enc.writeString("string");
    enc.writeString("datetime");
    enc.writeEOR();
    return 1;    
   }
   
   
  public int fp_write(java.sql.Connection con,Encoder enc,String method) throws Exception
   {
    throw new Exception("Not implemented");
   }

  public long fp_cachedLastModified() throws Exception
   {
    return -1;
   }

  public long fp_getLastModified(java.sql.Connection con) throws Exception
   {
    throw new Exception("Not implemented");
   }
 }

