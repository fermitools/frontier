/**
 * Manage the Frontier response payload
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import java.sql.*;
import java.io.*;
import gov.fnal.frontier.codec.*;
import gov.fnal.frontier.fdo.*;
import java.util.*;
import jakarta.servlet.ServletOutputStream;
import javax.crypto.Cipher;

public class Payload
 {
  private static Hashtable<String,FrontierDataObject> htFdo=new Hashtable<String,FrontierDataObject>(); 
  
  private DbConnectionMgr dbm;
  private Command cmd;
  private FrontierDataObject fdo;
  private Encoder enc;
  private boolean sign;
  private boolean emptyiserror;
  
  public boolean noCache=false;
  public long time_expire;
  public String type;
  public String version;
  public String encoder;
  public int err_code;
  public String err_msg;
  public int rec_num;
  public long full_size;
  
  protected Payload(Command a_cmd,DbConnectionMgr a_dbm) throws Exception
   {
    dbm=a_dbm;
    cmd=a_cmd;        
    //System.out.println("New payload for cmd="+cmd);
    type=cmd.obj_name;
    version=cmd.obj_version;
    encoder=cmd.encoder;
    err_code=0;
    err_msg="";
    rec_num=0;
    full_size=0;

    String sec=cmd.fds.getOptionalString("sec");
    if((sec!=null)&&(sec.equals("sig")))sign=true;
    else sign=false;

    String ttl=cmd.fds.getOptionalString("ttl");
    // Empty results are never allowed for forever cache
    // This doesn't affect caching time (since empty results are cached
    //   only a short time like errors) but it is an extra sanity check
    if((ttl!=null)&&(ttl.equals("forever")))emptyiserror=true;
    else emptyiserror=false;
        
    String key="_fdo__"+cmd.obj_name+"_@<:>@__"+cmd.obj_version;
    
    // BUILTINS
    if((cmd.obj_name.equals("frontier_request")&&cmd.obj_version.equals("1"))||
      (cmd.obj_name.equals("frontier_file")&&cmd.obj_version.equals("1")))
     {
      fdo=new PluginDataObject(dbm,cmd.obj_name,cmd.obj_version,cmd.fds,true);
      MethodDesc md=fdo.fdo_getMethodDesc(cmd.method);
      time_expire=md.getExpire();
      //System.out.println("Time_expire="+time_expire);      
      noCache=md.isNoCache();
      return;
     }
    // END BUILTINS
    
    // there are no non-builtins anymore
    throw new Exception("Unsupported command: "+cmd.obj_name);
   }
   
  public void start(ServletOutputStream out) throws Exception
   {
    fdo.fdo_start(out);
   }

  public long cachedLastModified() throws Exception
   {
    return fdo.fdo_cachedLastModified();
   }

  public long getLastModified(ServletOutputStream out,long if_modified_since) throws Exception
   {
    return fdo.fdo_getLastModified(out,if_modified_since);
   }

  public void send(ServletOutputStream out) throws Exception
   {
    rec_num=0;

    if ((cmd.encoder.length() < 4) ||
	(!cmd.encoder.substring(0,4).equals("BLOB")))
     {
      throw new Exception("Unsupported encoder: "+cmd.encoder);
     }

    String encparam = "";
    if (cmd.encoder.length() > 4)
	encparam=cmd.encoder.substring(4);
    
    switch(cmd.cmd_domain)
     {
      case Command.CMD_GET:
       enc=new BlobTypedEncoder(out,encparam,sign?cmd.query:null);
       try { 
	   rec_num=fdo.fdo_get(enc,cmd.method,out); 
	   // System.out.println("Number of records after fdo_get: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
	 full_size=enc.getOutputSize();
         Frontier.Log("rows="+rec_num+", full size="+full_size);
        }
       if(emptyiserror&&(rec_num==0))
	 throw new Exception("Empty result from query not allowed");
       return;
      
      case Command.CMD_META:
       enc=new BlobTypedEncoder(out,encparam,null);
       try { 
	   rec_num=fdo.fdo_meta(enc,cmd.method); 
	   // System.out.println("Number of records after fdo_meta: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
        }
       return;

      case Command.CMD_WRITE:
       enc=new BlobTypedEncoder(out,encparam,null);
       try { 
	   rec_num=fdo.fdo_write(enc,cmd.method,out); 
	   // System.out.println("Number of records after fdo_write: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
        }
       return;       
              
      default:
       throw new Exception("Unsupported command domain "+cmd.cmd_domain);
     }
   }
   
   
  public String getCheck() throws Exception
   {
    if(enc==null)
      return "";
    byte[] digest=enc.getMessageDigest();
    if(sign)
     {
      if(Frontier.private_key==null)
        throw new Exception("No private key available for signing");
      final Cipher cipher=Cipher.getInstance("RSA");
      cipher.init(Cipher.ENCRYPT_MODE,Frontier.private_key);
      byte[] encsig=cipher.doFinal(digest);
      byte[] b64sig=Base64Coder.encode(encsig);
      String sig=new String(b64sig);
      return "sig=\""+sig+"\"";
     }
    else
     {
      StringBuffer md5_ascii=new StringBuffer("");
      for(int i=0;i<digest.length;i++) 
       {
	int v=(int)digest[i];
	if(v<0)v=256+v;
	String str=Integer.toString(v,16);
	if(str.length()==1)md5_ascii.append("0");
	md5_ascii.append(str);
       }
      return "md5=\""+md5_ascii.toString()+"\"";
     }
   }  

  public void close(ServletOutputStream sos) throws Exception
   {
    fdo.fdo_close(sos);
   }
 }
