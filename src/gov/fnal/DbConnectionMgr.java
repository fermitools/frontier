package gov.fnal;

import javax.naming.*;

import java.sql.*;
import javax.sql.*;

import java.lang.Exception.*;

/**
 * Singleton class which provides database connections.  The specific
 * database supported is determined by configuring the server.xml file.
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class DbConnectionMgr {

    private static DbConnectionMgr instance;
    private DataSource dataSource = null;

    public static synchronized DbConnectionMgr getDbConnectionMgr() {
	if (instance == null) 
	    instance = new DbConnectionMgr();
	return instance;
    }

    private DbConnectionMgr() {}

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
	
    public Connection acquire() throws DbConnectionMgrException {
	Connection connection = null;
	try {
	    connection = dataSource.getConnection();
	} catch (SQLException e) {
		throw new DbConnectionMgrException(e.getMessage());
	}
	return connection;
    }

    public void release(Connection dbConnection) throws DbConnectionMgrException {
	try {
	    if (dbConnection != null)
		dbConnection.close();
	} catch (SQLException e) {
	    throw new DbConnectionMgrException(e.getMessage());
	}
    }
}

/**
 * Base exception which for the DbConnectionMgr class.
 */
class DbConnectionMgrException extends Exception {
    public DbConnectionMgrException () {}
    public DbConnectionMgrException (String message) {
	super(message);
    }
}

