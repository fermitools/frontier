package gov.fnal.frontier;

import java.io.*;
import gov.fnal.frontier.codec.Encoder;
import java.util.*;
import java.sql.*;



public class Xsd2DataObject implements FrontierDataObject
 {
  private DbConnectionMgr dbm;
  
  // XML related helper
  private Xsd2DataObjectHelper xsdhelper;
  
  // Public stuff
  public String obj_name;
  public String obj_version;
  public static final String xsd_version="2";

  
  protected Xsd2DataObject(DbConnectionMgr dbm,String requested_name,String requested_version) throws Exception
   {
    System.out.println("Xsd2DataObject()");
    this.dbm=dbm;
    obj_name=requested_name;
    obj_version=requested_version;
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
     
   
  public int fdo_get(Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    System.out.println("Xsd2DataObject.fdo_get()");
    int rec_num=0;
    
    MethodDesc md=xsdhelper.getMethodDesc(method);  
    String sql=md.sql_str;
    System.out.println("sql ["+sql+"]");
    
    Connection con=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    
    try
     {
      con=dbm.acquire();
      stmt=con.prepareStatement(sql);
      for(int i=0;i<md.aParams.size();i++)
       {
        String[] param=(String[])md.aParams.get(i);
        String val=fds.getString(param[1]);
        System.out.println("Param "+i+" ["+param[1]+":"+val+"]");
        stmt.setString(i+1,val);
       }
      rs=stmt.executeQuery();
      while(rs.next())
       {
        for(int i=0;i<xsdhelper.aFields.size();i++)
         {
          String[] field=(String[])xsdhelper.aFields.get(i);
          switch(field[0].charAt(0))
           {
            case 'i': enc.writeInt(rs.getInt(i+1)); break;
            case 'l': enc.writeLong(rs.getLong(i+1)); break;
            case 'd': enc.writeDouble(rs.getDouble(i+1)); break;
            case 'f': enc.writeFloat(rs.getFloat(i+1)); break;
            case 's': enc.writeString(rs.getString(i+1)); break;
            case 'D': enc.writeDate(rs.getDate(i+1)); break;
            case 'b': 
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
      if(con!=null) try{dbm.release(con);}catch(Exception e){}
     }
    return rec_num;
   }
   
   
  public int fdo_meta(Encoder enc) throws Exception
   {
    System.out.println("Xsd2DataObject.fdo_meta()");
    for(int i=0;i<xsdhelper.aFields.size();i++)
     {
      String[] field=(String[])xsdhelper.aFields.get(i);
      enc.writeString(field[1]);
      enc.writeString(field[2]);
     }
    enc.writeEOR();
    return 1; // Just one record
   }
 }
