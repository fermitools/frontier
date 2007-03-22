package gov.fnal.frontier;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import gov.fnal.frontier.codec.*;
import java.sql.*;
import com.jcraft.jzlib.*;

public class SQLPlugin implements FrontierPlugin
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
   
   
  public int fp_get(java.sql.Connection con,Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);
    
    String param=fds.getString("p1");
    //System.out.println("Got param ["+param+"]");
    
    StringBuffer sb=new StringBuffer(param);
    int len=param.length();
    for(int i=0;i<len;i++)
     {
      char ch=param.charAt(i);
      switch(ch)
       {
        case '.': sb.setCharAt(i,'+'); break;
        case '-': sb.setCharAt(i,'/'); break;
        case '_': sb.setCharAt(i,'='); break;
        default:
       }
     }
    //sb.append('\n'); //needed with Base64.decode, not Base64Coder.decode
    param=sb.toString();
    //System.out.println("Pre param ["+param+"]");
    
    byte[] bascii=param.getBytes("US-ASCII");
    
    //byte[] bbin=Base64.decode(bascii,0,bascii.length);
    byte[] bbin=Base64Coder.decode(bascii);
    
    byte[] buffer=new byte[2048];    
    java.io.ByteArrayOutputStream baos=new java.io.ByteArrayOutputStream();
    java.io.ByteArrayInputStream bais=new java.io.ByteArrayInputStream(bbin);
    //Sun's GZIP/ZIP sucks...
    //java.util.zip.ZipInputStream gzis=new java.util.zip.ZipInputStream(bais);
    //Use JZip
    ZInputStream gzis=new ZInputStream(bais);

    while((len=gzis.read(buffer,0,buffer.length))>=0)
     {
      baos.write(buffer,0,len);
     }
    byte[] bsql=baos.toByteArray();    
    
    String sql=new String(bsql,"US-ASCII");
    Frontier.Log("SQL ["+sql+"]");

    String stmp=sql.toLowerCase();
    if(stmp.indexOf("drop ")>=0 ||
       stmp.indexOf("delete ")>=0 ||
       stmp.indexOf("insert ")>=0 ||
       stmp.indexOf("alter ")>=0 ||
       stmp.indexOf("create ")>=0) throw new Exception("Query cancelled");
    
    Statement stmt=null;
    ResultSet rs=null;
    int row_count=0;
    try
     {
      stmt=con.createStatement();
      stmt.setFetchSize(100); /* huge performance boost for small rows */
      			      /* causes much better row prefetching */
			      /* 1000 & 10000 are slightly faster but cause
			         executeQuery to abort with OutOfMemoryError
				 for some queries with larger rows. */
      rs=stmt.executeQuery(sql);
      ResultSetMetaData rsmd=rs.getMetaData();
      int cnum=rsmd.getColumnCount();
      
      for(int i=1;i<=cnum;i++)
       {
        String n=rsmd.getColumnName(i);
        String t=rsmd.getColumnTypeName(i);
        if(t=="NUMBER")
         {
          // append column precision to column type name if non-zero
	  // this was requested by Luis Ramos at CERN
          int colPrec=rsmd.getPrecision(i);
          if(colPrec!=0)
            t+="("+colPrec+")";
	 }
        enc.writeString(n);
        enc.writeString(t);
       }
      enc.writeEOR();
            
      while(rs.next()) {
	row_count++;
        for(int i=1;i<=cnum;i++)
         {
          String t=rsmd.getColumnTypeName(i);
	  if(t=="BLOB")
	   {
	    Blob blob=rs.getBlob(i);
	    //byte[] b=blob.getBytes((long)1,(int)blob.length());
	    //enc.writeBytes(b);
	    if(blob!=null)
	      enc.writeStream(blob.getBinaryStream(),(int)blob.length());
	    else
	      enc.writeString("");
	   }
	  else
	   {
            String s=rs.getString(i);
            enc.writeString(s);
	   }
         }
        enc.writeEOR();
       }
     }
    finally
     {
      enc.flush();
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
     }        
    return row_count;
   }

      
  public int fp_meta(Encoder enc,String method) throws Exception
   {
    throw new Exception("META methods are not supported by this object");
   }
   
   
  public int fp_write(java.sql.Connection con,Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    throw new Exception("Not implemented");
   }
 }

