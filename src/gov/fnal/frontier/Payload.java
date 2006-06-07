package gov.fnal.frontier;

import java.sql.*;
import java.io.*;
import gov.fnal.frontier.codec.*;
import gov.fnal.frontier.fdo.*;
import java.util.*;

public class Payload
 {
  private static Hashtable htFdo=new Hashtable(); 
  
  private DbConnectionMgr dbm;
  private Command cmd;
  private FrontierDataObject fdo;
  
  private static final String fds_sql=
   "select xsd_type,xsd_data from "+Frontier.getXsdTableName()+" where name = ? and version = ? ";
   
  public boolean noCache=false;
  public long time_expire;
  public String type;
  public String version;
  public String encoder;
  public int err_code;
  public String err_msg;
  public String md5;
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
    md5="";
    rec_num=0;
    full_size=0;
        
    String key="_fdo__"+cmd.obj_name+"_@<:>@__"+cmd.obj_version;
    
    if(Frontier.isFdoCache()) fdo=(FrontierDataObject)htFdo.get(key);
    if(fdo!=null)
     {
      //System.out.println("Got "+key+" from cache");
      MethodDesc md=fdo.fdo_getMethodDesc(cmd.method);
      time_expire=md.getExpire();
      noCache=md.isNoCache();
      return;      
     }
     
    // BUILTINS
    if(cmd.obj_name.equals("frontier_request") && cmd.obj_version.equals("1"))
     {
      fdo=new PluginDataObject(dbm,cmd.obj_name,cmd.obj_version,true);
      //fdo_init() MUST NOT BE CALLED FOR BUILTINS!
      MethodDesc md=fdo.fdo_getMethodDesc(cmd.method);
      time_expire=md.getExpire();
      //System.out.println("Time_expire="+time_expire);      
      noCache=md.isNoCache();
      if(Frontier.isFdoCache()) htFdo.put(key,fdo);
      return;
     }
    // END BUILTINS
    
    Connection con=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    try
     {
      con=dbm.getDescriptorConnection();
      stmt=con.prepareStatement(fds_sql);
      stmt.setString(1,cmd.obj_name);
      stmt.setString(2,cmd.obj_version);
      rs=stmt.executeQuery();
      if(!rs.next()) throw new Exception("Object "+cmd.obj_name+":"+cmd.obj_version+" does not exists");
      String xsd_type=rs.getString("xsd_type");      
      Blob blob=rs.getBlob("xsd_data");
      int len=(int)blob.length();
      byte[] b=blob.getBytes((long)1,len);
      
      if(xsd_type.equals("xml"))       fdo=new XsdDataObject(dbm,cmd.obj_name,cmd.obj_version);
      else if(xsd_type.equals("xsd2")) fdo=new Xsd2DataObject(dbm,cmd.obj_name,cmd.obj_version);
      else if(xsd_type.equals("plugin")) fdo=new PluginDataObject(dbm,cmd.obj_name,cmd.obj_version);
      else                             throw new Exception("Unsupported xsd_type "+xsd_type+".");
      
      fdo.fdo_init(b);
      MethodDesc md=fdo.fdo_getMethodDesc(cmd.method);
      time_expire=md.getExpire();
      //System.out.println("Time_expire="+time_expire);      
      noCache=md.isNoCache();
      if(Frontier.isFdoCache()) htFdo.put(key,fdo);      
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
      if(con!=null) try{dbm.release(con);}catch(Exception e){}
     }     
   }
   
   
  public void send(OutputStream out) throws Exception
   {
    Encoder enc=null;
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
       enc=new BlobTypedEncoder(out,encparam);
       try { 
	   rec_num=fdo.fdo_get(enc,cmd.method,cmd.fds); 
	   System.out.println("Number of records after fdo_get: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
         md5=md5Digest(enc);
	 full_size=enc.getOutputSize();
        }
       return;
      
      case Command.CMD_META:
       enc=new BlobTypedEncoder(out,encparam);
       try { 
	   rec_num=fdo.fdo_meta(enc,cmd.method); 
	   System.out.println("Number of records after fdo_meta: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
         md5=md5Digest(enc);
        }
       return;

      case Command.CMD_WRITE:
       enc=new BlobTypedEncoder(out,encparam);
       try { 
	   rec_num=fdo.fdo_write(enc,cmd.method,cmd.fds); 
	   System.out.println("Number of records after fdo_write: " + rec_num);
       }
       finally 
        { 
         enc.close(); 
         md5=md5Digest(enc);
        }
       return;       
              
      default:
       throw new Exception("Unsupported command domain "+cmd.cmd_domain);
     }
   }
   
   
  private String md5Digest(Encoder encoder) throws Exception
   {
    StringBuffer md5_ascii=new StringBuffer("");
    byte[] md5_digest=encoder.getMD5Digest();
    for(int i=0;i<md5_digest.length;i++) 
     {
      int v=(int)md5_digest[i];
      if(v<0)v=256+v;
      String str=Integer.toString(v,16);
      if(str.length()==1)md5_ascii.append("0");
      md5_ascii.append(str);
     }
    return md5_ascii.toString();
   }  
 }
