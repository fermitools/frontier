package gov.fnal.frontier;

import java.io.*;
import gov.fnal.frontier.fdo.*;
import java.util.*;
import java.sql.*;
import javax.servlet.ServletOutputStream;



public class Xsd2DataObject implements FrontierDataObject
 {
  private DbConnectionMgr dbm;
  
  private FrontierDataStream frontier_data_stream;
  
  // XML related helper
  private Xsd2DataObjectHelper xsdhelper;
  
  // Public stuff
  public String obj_name;
  public String obj_version;
  public static final String xsd_version="2";

  
  protected Xsd2DataObject(DbConnectionMgr dbm,String requested_name,String requested_version,FrontierDataStream fds) throws Exception
   {
    System.out.println("Xsd2DataObject()");
    this.dbm=dbm;
    obj_name=requested_name;
    obj_version=requested_version;
    frontier_data_stream=fds;
    xsdhelper=null;
   }

         
  public void fdo_init(byte[] body) throws Exception
   {
    xsdhelper=new Xsd2DataObjectHelper(obj_name,obj_version);
    xsdhelper.init(body);
   }
 
   
  public MethodDesc fdo_getMethodDesc(String method) throws Exception
   {
    MethodDesc md=xsdhelper.getMethodDesc(method);
    return md;
   }
     
  public void fdo_start(ServletOutputStream sos) throws Exception
   {
   }
   
  public int fdo_get(Encoder enc,String method,ServletOutputStream sos) throws Exception
   {
    System.out.println("Xsd2DataObject.fdo_get()");
    int rec_num=0;
    
    Xsd2DataObjectHelper.MethodInfo mi=xsdhelper.getMethodInfo(method);  
    // SQL type=="update"
    if(mi.sql_type.equals("update")) throw new Exception("Updates are not supported in GET domain");    
    // SQL type=="call"
    if(mi.sql_type.equals("call")) throw new Exception("PL/SQL calls are not supported yet"); 
    
    String sql=mi.sql_str;
    System.out.println("sql ["+sql+"]");
    
    Connection con=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    
    try
     {
      con=dbm.acquire(sos);
      stmt=con.prepareStatement(sql);
      for(int i=0;i<mi.aParams.size();i++)
       {
        FieldDesc param=(FieldDesc)mi.aParams.get(i);
        String val=frontier_data_stream.getString(param.n);
        System.out.println("Param "+i+" ["+param.n+":"+val+"]");
        stmt.setString(i+1,val);
       }
      rs=stmt.executeQuery();
      while(rs.next())
       {
        for(int i=0;i<xsdhelper.aFields.size();i++)
         {
          FieldDesc field=(FieldDesc)xsdhelper.aFields.get(i);
          switch(field.t)
           {
            case FieldDesc.F_INT:    enc.writeInt(rs.getInt(i+1)); break;
            case FieldDesc.F_LONG:   enc.writeLong(rs.getLong(i+1)); break;
            case FieldDesc.F_DOUBLE: enc.writeDouble(rs.getDouble(i+1)); break;
            case FieldDesc.F_FLOAT:  enc.writeFloat(rs.getFloat(i+1)); break;
            case FieldDesc.F_STRING: enc.writeString(rs.getString(i+1)); break;
            case FieldDesc.F_BYTES:  enc.writeBytes(rs.getBytes(i+1)); break;  //bytes aka RAW
            case FieldDesc.F_DATE:   enc.writeDate(rs.getDate(i+1)); break;
            case FieldDesc.F_BLOB: 
             Blob blob=rs.getBlob(i+1);
             int len=(int)blob.length();
             byte[] b=blob.getBytes((long)1,len);
             enc.writeBytes(b);
             break;
            default: throw new Exception("Type adjustment is needed");
           }
         }
        rec_num++;
        enc.writeEOR();
       }
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
      if(con!=null) try{dbm.release(con,sos);}catch(Exception e){}
     }
    return rec_num;
   }
   
   
  public int fdo_meta(Encoder enc,String method) throws Exception
   {    
    Xsd2DataObjectHelper.MethodInfo mi=xsdhelper.getMethodInfo(method);
    System.out.println("Xsd2DataObject.fdo_meta(), sql_type ["+mi.sql_type+"]");
    if(mi.sql_type.equals("update"))
     {
      enc.writeString("int");
      enc.writeString("rows");
      enc.writeEOR();
      return 1;
     }
     
    if(mi.sql_type.equals("call")) throw new Exception("PL/SQL calls are not supported yet");

    // SQL type=="query"
    for(int i=0;i<xsdhelper.aFields.size();i++)
     {
      FieldDesc field=(FieldDesc)xsdhelper.aFields.get(i);
      enc.writeString(FieldDesc.type_name[field.t]);
      enc.writeString(field.n);
     }
    enc.writeEOR();
    return 1; // Just one record
   }
   
   
  public int fdo_write(Encoder enc,String method,ServletOutputStream sos) throws Exception   
   {
    System.out.println("Xsd2DataObject.fdo_write()");
    int rec_num=0;
    
    Xsd2DataObjectHelper.MethodInfo mi=xsdhelper.getMethodInfo(method);  
    // SQL type=="call"
    if(mi.sql_type.equals("call")) throw new Exception("PL/SQL calls are not supported yet");        
    // SQL type=="query"
    if(mi.sql_type.equals("query")) throw new Exception("Queries are not supported in WRITE domain");              
    
    String sql=mi.sql_str;
    System.out.println("sql ["+sql+"]");
    
    Connection con=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    
    try
     {
      con=dbm.acquire(sos);
      stmt=con.prepareStatement(sql);
      for(int i=0;i<mi.aParams.size();i++)
       {
        FieldDesc param=(FieldDesc)mi.aParams.get(i);
        String val=frontier_data_stream.getString(param.n);
        System.out.println("Param "+i+" ["+param.n+":"+val+"]");
        stmt.setString(i+1,val);
       }
      int rows=stmt.executeUpdate();
      enc.writeInt(rows);
      rec_num=1; 
      enc.writeEOR();
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
      if(con!=null) try{dbm.release(con,sos);}catch(Exception e){}
     }
    return rec_num;
   }

  public long fdo_cachedLastModified()
   {
    return -1;
   }

  public long fdo_getLastModified(ServletOutputStream sos)
   {
    return 0;
   }

  public void fdo_close(ServletOutputStream sos)
   {
   }
 }
