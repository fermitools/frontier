/**
 * Plugin data object interface
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
import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import java.util.*;
import jakarta.servlet.ServletOutputStream;


public class PluginDataObject implements FrontierDataObject
 {
  private DbConnectionMgr dbm=null;
  private FrontierPlugin plugin;
  private boolean acquired;
  
  protected FrontierDataStream frontier_data_stream;

  public String obj_name;
  public String obj_version;
  
  
  protected PluginDataObject(DbConnectionMgr dbm,String builtin_name,String builtin_version,FrontierDataStream fds,boolean is_plugin) throws Exception
   {
    this.dbm=dbm;
    obj_name=builtin_name;
    obj_version=builtin_version;
    if(builtin_name.equals("frontier_request"))
     {
      //System.out.println("PluginDataObject(builtin:frontier_request)");
      plugin=new SQLPlugin(fds);
      return;
     }
    if(builtin_name.equals("frontier_file"))
     {
      //System.out.println("PluginDataObject(builtin:frontier_file)");
      plugin=new FilePlugin(fds);
      return;
     }
    throw new Exception("Unknown builtin plugin "+builtin_name+":"+builtin_version);
   }
      
  
  protected PluginDataObject(DbConnectionMgr dbm,String requested_name,String requested_version,FrontierDataStream fds) throws Exception
   {
    //System.out.println("PluginDataObject()");
    this.dbm=dbm;
    obj_name=requested_name;
    obj_version=requested_version;
    plugin=null;
    frontier_data_stream=fds;
   }

         
  public MethodDesc fdo_getMethodDesc(String method) throws Exception
   {
    MethodDesc md=plugin.fp_getMethodDesc(method);
    return md;
   }
     
  private void acquireConnectionIfNeeded(ServletOutputStream sos) throws Exception
   {
    if(!acquired)
     {
      dbm.acquire(plugin,sos);
      acquired=true;
     }
   }

  public void fdo_start(ServletOutputStream sos) throws Exception
   {
    acquireConnectionIfNeeded(sos);
   }
   
  public int fdo_get(Encoder enc,String method,ServletOutputStream sos) throws Exception
   {
    //System.out.println("PluginDataObject.fdo_get()");
    int rec_num=0;
    
    acquireConnectionIfNeeded(sos);
    rec_num=plugin.fp_get(dbm,enc,method);
    return rec_num;
   }
   
   
  public int fdo_meta(Encoder enc,String method) throws Exception
   {    
    //System.out.println("PluginDataObject.fdo_meta()");
    int rec_num;
    
    rec_num=plugin.fp_meta(enc,method);
    return rec_num;
   }
   
   
  public int fdo_write(Encoder enc,String method,ServletOutputStream sos) throws Exception   
   {
    //System.out.println("PluginDataObject.fdo_write()");
    int rec_num=0;
    
    acquireConnectionIfNeeded(sos);
    rec_num=plugin.fp_write(enc,method);
    return rec_num;
   }
   
  public long fdo_cachedLastModified() throws Exception
   {
    return plugin.fp_cachedLastModified();
   }

  public long fdo_getLastModified(ServletOutputStream sos,long if_modified_since) throws Exception
   {
    acquireConnectionIfNeeded(sos);
    return plugin.fp_getLastModified(if_modified_since);
   }

  public void fdo_close(ServletOutputStream sos) throws Exception
   {
    if(acquired)
     {     
      acquired=false;
      try
       {
	dbm.release(plugin,sos);
       }
      catch(Exception e)
       {
	Frontier.Log("Error releasing DB connection "+e);
	throw e;
       }
     }
   }
 }
