package gov.fnal.frontier;

import java.io.*;
import java.sql.*;
import javax.sql.*;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class ServicerFactory {

    private DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();
    private XmlLoader xmlLoader = new XmlLoader();

    private class SFDataResult {
	public String      type = null;
	public BufferedInputStream data = null;
	
	SFDataResult(String aType, BufferedInputStream aData) {
	    type = aType;
	    data = aData;
	}
    }

    ServicerFactory() {}

    public Servicer load(String className, String classVersion) 
	throws ServicerFactoryException {
	Servicer servicer = null;
	/**
	   This is where we will put a call to check the cache
	**/
	SFDataResult result = getData(className,classVersion);
	try {
	    if (result.type.compareToIgnoreCase("xml") == 0) {
		XmlDescriptor descriptor = (XmlDescriptor) xmlLoader.load(className,classVersion,result.data);
		servicer = new XmlServicer(descriptor);
	    }
	    else if (result.type.compareToIgnoreCase("jar") == 0)
		throw new ServicerFactoryException("Jar files are not yet supported");
	    else
		throw new ServicerFactoryException("Received an unknown type of " + result.type);
	} catch (LoaderException e) {
	    throw new ServicerFactoryException(e.getMessage());
	}
	return servicer;
    }

    private SFDataResult getData(String className, String classVersion)
	throws ServicerFactoryException {
	SFDataResult result = null;
	String query = "select type, data from frontier_descriptor where name = '" + className + "'";
	query += " and version = '" + classVersion + "'";
	Connection connection = null;
	Statement stmt = null;
	ResultSet rs = null;
	try {
	    connection = connMgr.acquire();
	    stmt = connection.createStatement();
	    rs = stmt.executeQuery(query);
	    long recordCnt = 0;
	    while (rs.next()) {
		if (recordCnt > 0) 
		    throw new ServicerFactoryException("Multiple rows obtained for query. " + query);
		Blob blob = rs.getBlob(2);

		result = new SFDataResult(rs.getString(1),
					  new BufferedInputStream(blob.getBinaryStream()));
		recordCnt++;
	    }
	    if (recordCnt == 0) 
		throw new ServicerFactoryException("No rows obtained for query. " + query);
	} catch (DbConnectionMgrException e) {
	    throw new ServicerFactoryException(e.getMessage());
	} catch (SQLException e) {
	    throw new ServicerFactoryException(e.getMessage());
	} finally {
	    try {if(rs!=null)rs.close();} catch (SQLException e) {}
	    try {if(stmt!=null)stmt.close();} catch (SQLException e) {}
	    try {connMgr.release(connection);} catch (DbConnectionMgrException e) {}
	}
	return result;
    }

}
