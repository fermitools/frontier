package gov.fnal.frontier.util.xsdloader;

// $Id$

import java.sql.*;
import java.io.*;


public class Main
 {
  public static final String sql1=
   "select name from frontier_descriptors where name = ? and version = ?";
   
  public static final String sql2=
   "insert into frontier_descriptors "+
   "(name,version,xsd_type,xsd_data,create_date,create_user,update_date,update_user) "+
   "values "+
   "(?, ?, ?, ?, SYSDATE, ?, NULL, NULL)";
   
  public static final String sql3=
   "update frontier_descriptors set "+
   "xsd_type = ?, xsd_data = ?, update_date=SYSDATE, update_user = ? "+
   "where name = ? and version = ? ";
   
  public Main()
   {
   }
 
   
  public static void main(String[] argv)
   {   
    System.out.println("XSD Loader v1.0");
    
    if(argv.length<4 || argv.length>5)
     {
      System.out.println("Usage: java gov.fnal.frontier.util.xsdloader.Main db-url db-user db-passwd list-file-name [commit]");
      return;
     }
    
    Main main=new Main(); 
    main.doit(argv);
   }
   
   
  private void doit(String[] argv)
   {
    Connection con=null;
    PreparedStatement stmt1=null;
    PreparedStatement stmt2=null;
    PreparedStatement stmt3=null;
    ResultSet rs=null;
    int count=0;
    int cnt_update=0;
    int cnt_new=0;
    try
     {
      DriverManager.registerDriver(new oracle.jdbc.OracleDriver());
      con=DriverManager.getConnection("jdbc:oracle:thin:@"+argv[0],argv[1],argv[2]);
      con.setAutoCommit(false);

      stmt1=con.prepareStatement(sql1);
      stmt2=con.prepareStatement(sql2);
      stmt3=con.prepareStatement(sql3);
            
      BufferedReader br=new BufferedReader(new FileReader(argv[3]));
      String xsd_name=br.readLine();
      while(xsd_name!=null)
       {
        ++count;
        System.out.println("\n"+count+" processing "+xsd_name+"...");
        File f=new File(xsd_name);
        byte[] buf=new byte[(int)f.length()];
        FileInputStream fis=new FileInputStream(f);
        int ret=fis.read(buf);
        if(ret!=buf.length) throw new Exception("Can not read. Abort.");
        
        Xsd xsd=new Xsd();
        xsd.init(buf);
        
        XsdValidator validator=null;        
        if(xsd.xsd_version.equals("1")) validator=new XsdV1Validator(xsd.obj_name,xsd.obj_version);
        else if(xsd.xsd_version.equals("2")) validator=new XsdV2Validator(xsd.obj_name,xsd.obj_version);
        else throw new Exception("Unsupported XSD version "+xsd.xsd_version);
        System.out.println("Validating...");
        validator.init(buf);
        
        ByteArrayInputStream bains=new ByteArrayInputStream(buf,0,buf.length);
        
        boolean entry_exists;
        stmt1.setString(1,xsd.obj_name);
        stmt1.setString(2,xsd.obj_version);
        rs=stmt1.executeQuery();
        entry_exists=rs.next();
        rs.close(); rs=null;
        
        if(entry_exists)
         {
          System.out.println("Entry exists, updating...");
          ++cnt_update;
          stmt3.setString(1,xsd.xsd_version.equals("1")?"xml":"xsd2");
          stmt3.setBinaryStream(2,bains,buf.length);
          stmt3.setString(3,argv[1]);
          stmt3.setString(4,xsd.obj_name);
          stmt3.setString(5,xsd.obj_version);
          stmt3.execute();
         }
        else
         {
          System.out.println("New entry, loading...");
          ++cnt_new;
          stmt2.setString(1,xsd.obj_name);
          stmt2.setString(2,xsd.obj_version);
          stmt2.setString(3,xsd.xsd_version.equals("1")?"xml":"xsd2");
          stmt2.setBinaryStream(4,bains,buf.length);
          stmt2.setString(5,argv[1]);
          stmt2.execute();
         }
        System.out.println("Done.");
        
        xsd_name=br.readLine();
       }
       
      System.out.println("");
             
      if(argv.length==5 && argv[4].equals("commit"))
       { 
        System.out.print("!!! Committing...");      
        con.commit();
        System.out.println(" Done.");
       }
      else
       {
        con.rollback();        
        System.out.println("!!! Validate-only mode - THE TRANSACTION HAS BEEN ROLLED BACK.");
        System.out.println("!!! Use \"commit\" command line argument to request actual write");
       }
      System.out.println("\nTotal "+count+", updated "+cnt_update+", created "+cnt_new+".");
     }
    catch(Exception e)
     {
      if(con!=null)try{con.rollback();}catch(Exception ee){}
      System.out.println("Error: "+e);
      e.printStackTrace();
     }
    finally
     {
      if(rs!=null)try{rs.close();}catch(Exception e){}
      if(stmt1!=null)try{stmt1.close();}catch(Exception e){}
      if(stmt2!=null)try{stmt2.close();}catch(Exception e){}
      if(stmt3!=null)try{stmt3.close();}catch(Exception e){}      
      if(con!=null)try{con.close();}catch(Exception e){}
     }
   }
   
   
  public static String readEntry(String prompt)
   {
    try
     {
      StringBuffer buffer=new StringBuffer();
      System.out.print(prompt);
      System.out.flush();
      int c=System.in.read();
      while(c!='\n' && c!=-1)
       {
        buffer.append((char)c);
        c=System.in.read();
       }
      return buffer.toString().trim();
     }
    catch(Exception e)
     {
      return "";
     }
   }   
 }
