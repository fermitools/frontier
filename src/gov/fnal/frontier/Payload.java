package gov.fnal.frontier;

import java.sql.*;

public class Payload
 {
  private DbConnectionMgr dbm;
  private Command cmd;
  private FrontierDataObject fdo;
  
  private static finale String fds_sql=
   "select xsd_type,xsd_data from "+Frontier.getXsdTableName()+" where name = ? and version = ? ";
  
  protected Payload(Command a_cmd,DbConnectionMgr a_dbm) throws Exception
   {
    dbm=a_dbm;
    cmd=a_cmd;
    
    System.out.println("New payload for cmd="+cmd);
    
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
      rs.next();
      String xsd_type=rs.getString("xsd_type");
      if(!xsd_type.equals("xml")) throw new Exception("Unsupported xsd_type "+xsd_type+".");
      Blob blob=rs.getBlob("xsd_data");
      int len=(int)blob.length();
      byte[] b=blob.getBytes((long)1,len);
      fdo=new XsdDataObject(dbm);
      fdo.fdo_init(b);
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
      if(con!=null) try{dbm.release(con);}catch(Exception e){}
     }
   }
 }
