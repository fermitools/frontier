package gov.fnal.frontier;

import javax.naming.*;

import java.sql.*;
import javax.sql.*;


/**
 * Singleton class which provides database connections.  The specific
 * database supported is determined by configuring the server.xml file.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class DbConnectionMgr {

    private static DbConnectionMgr instance;
    private DataSource dataSource = null;

    /**
     * Obtains the singleton instance of a DbConnectionMgr.  The instance is
     * created if it does not exist.
     * @return DbConnectionMgr
     */
    public static synchronized DbConnectionMgr getDbConnectionMgr() {
	if (instance == null)
	    instance = new DbConnectionMgr();
	return instance;
    }

    /**
     * Initializes the DbConnectionMgr with data from the context data.  (Tomcat
     * XML files.)
     * @throws DbConnectionMgrException
     */
    public synchronized void initialize() throws DbConnectionMgrException {
	if (dataSource == null) {
	    try {
		Context initContext = new InitialContext();
		Context envContext  = (Context)initContext.lookup("java:/comp/env");
		dataSource = (DataSource)envContext.lookup("jdbc/frontier");
	    } catch (NamingException e) {
		throw new DbConnectionMgrException(e.getMessage());
	    }
	}
    }

    /**
     * Obtains an active  connection to the database from the pool.  If one is
     * not available the request will be queued.
     * @throws DbConnectionMgrException
     * @return Connection
     */
    public Connection acquire() throws DbConnectionMgrException {
	Connection connection = null;
	try {
	    connection = dataSource.getConnection();
	} catch (SQLException e) {
		throw new DbConnectionMgrException(e.getMessage());
	}
	return connection;
    }

    /**
     * Returns a connection to the pool of available connections making it
     * available to other users.
     * @param dbConnection Connection
     * @throws DbConnectionMgrException
     */
    public void release(Connection dbConnection) throws DbConnectionMgrException {
	try {
	    if (dbConnection != null)
		dbConnection.close();
	} catch (SQLException e) {
	    throw new DbConnectionMgrException(e.getMessage());
	}
    }

    /**
     * Private constructor for the singleton instance.
     */
    private DbConnectionMgr() {}
}


