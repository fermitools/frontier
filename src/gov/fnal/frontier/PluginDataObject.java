package gov.fnal.frontier;

import java.io.*;
import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import java.util.*;
import java.sql.*;
import javax.servlet.ServletOutputStream;


public class PluginDataObject implements FrontierDataObject
 {
  private DbConnectionMgr dbm;
  private Connection acquiredConnection=null;
  
  // Public stuff
  public String obj_name;
  public String obj_version;
  private FrontierPlugin plugin;
  protected FrontierDataStream frontier_data_stream;
  
  
  protected PluginDataObject(DbConnectionMgr dbm,String builtin_name,String builtin_version,FrontierDataStream fds,boolean is_plugin) throws Exception
   {
    if(builtin_name.equals("frontier_request"))
     {
      //System.out.println("PluginDataObject(builtin:frontier_request)");
      this.dbm=dbm;
      obj_name=builtin_name;
      obj_version=builtin_version;
      plugin=new SQLPlugin(fds);
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

         
  public void fdo_init(byte[] body) throws Exception
   {
    String class_name=new String(body,"US-ASCII");
    Class c_p=Class.forName(class_name);
    plugin=(FrontierPlugin)c_p.newInstance();
   }
 
   
  public MethodDesc fdo_getMethodDesc(String method) throws Exception
   {
    MethodDesc md=plugin.fp_getMethodDesc(method);
    return md;
   }
     
  public void fdo_start(ServletOutputStream sos) throws Exception
   {
    if(acquiredConnection==null)acquiredConnection=dbm.acquire(sos);
   }
   
  public int fdo_get(Encoder enc,String method,ServletOutputStream sos) throws Exception
   {
    //System.out.println("PluginDataObject.fdo_get()");
    int rec_num=0;
    
    if(acquiredConnection==null)acquiredConnection=dbm.acquire(sos);
    rec_num=plugin.fp_get(acquiredConnection,dbm,enc,method);
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
    
    if(acquiredConnection==null)acquiredConnection=dbm.acquire(sos);
    rec_num=plugin.fp_write(acquiredConnection,enc,method);
    return rec_num;
   }
   
  public long fdo_cachedLastModified() throws Exception
   {
    return plugin.fp_cachedLastModified();
   }

  public long fdo_getLastModified(ServletOutputStream sos) throws Exception
   {
    if(acquiredConnection==null)acquiredConnection=dbm.acquire(sos);
    return plugin.fp_getLastModified(acquiredConnection);
   }

  public void fdo_close(ServletOutputStream sos) throws Exception
   {
    if(acquiredConnection!=null)
     {
      try
       {
        dbm.release(acquiredConnection,sos);
       }
      catch(Exception e)
       {
        Frontier.Log("Error releasing DB connection "+e);
	throw e;
       }
      acquiredConnection=null;
     }
   }
 }
