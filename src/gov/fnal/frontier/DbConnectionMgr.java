package gov.fnal.frontier;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.sql.Connection;
import java.sql.SQLException;
import javax.sql.DataSource;

/**
 * Singleton class which provides database connections.  The specific
 * database supported is determined by configuring the server.xml file.
 * @author Stephen P. White <swhite@fnal.gov>
 * udapted for Frontier3 by: Sergey Ksoyakov
 * @version $Revision$
 */
public class DbConnectionMgr 
 {
  private static DbConnectionMgr instance=null;
  private static Boolean mutex=new Boolean(true);
  private DataSource dataSource=null;

  public static DbConnectionMgr getDbConnectionMgr() throws Exception
   {
    if(instance!=null) return instance;
    
    synchronized(mutex)
     {
      if(instance!=null) return instance;      
      DbConnectionMgr m=new DbConnectionMgr();
      instance=m;
     }
    return instance;
   }

  
  private DbConnectionMgr() throws Exception
   {
    Context initContext=new InitialContext();
    Context envContext=(Context)initContext.lookup("java:/comp/env");
    System.out.println("Looking for ["+Frontier.getDsName()+"]");
    dataSource = (DataSource)envContext.lookup(Frontier.getDsName());
    //dataSource=(DataSource)initContext.lookup(Frontier.getDsName());
    if(dataSource==null) throw new Exception("DataSource ["+Frontier.getDsName()+"] not found");
   }

  
  public Connection acquire() throws Exception 
   {
    Connection connection;
    connection=dataSource.getConnection();
    return connection;
   }

 
  public void release(Connection dbConnection) throws Exception 
   {
    if(dbConnection!=null) dbConnection.close();
   }
   
   
  // This one to get frontier descriptors (or plugins), could have different DS
  public Connection getDescriptorConnection() throws Exception
   {
    Connection connection;
    connection=dataSource.getConnection();
    return connection;   
   }
 }
