package gov.fnal.frontier;

import gov.fnal.frontier.codec.*;
import java.util.*;
import java.io.*;
import java.sql.*;
import javax.sql.*;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class UniversalQueryRequestHandler extends RequestHandler {

    private static boolean LF       = true;
    private static boolean noLF     = false;

    private String  encoding        = null;
    private String  objectName      = null;
    private String  objectVersion   = null;
    private Encoder encoder         = null;
    private ClassLoader classLoader = new ClassLoader();
    private DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();


    UniversalQueryRequestHandler(ServletOutputStream writer) {
	super(writer);
    }
    
    public void process(Command command) throws ServletException {

	try {
	    validate(command);
	    encoder = getEncoder(writer);
	    Servicer servicer = classLoader.load(objectName,objectVersion);
	    servicer.validate(command);
	    stream("<payload type=\"" + objectName + "\" version=\"" + objectVersion,noLF);
	    stream("\" encoding=\"" + encoding + "\">",LF);
	    produceData(servicer);
	} catch (RequestHandlerException e) {
	    stream("<payload type=\"" + objectName + "\" version=\"" + objectVersion + "\"",noLF);
	    stream(" encoding=\"" + encoding + "\">",LF);
	    stream("<quality error=\"1\" code=\"???\" message=\"" + e.getMessage() + "\"/>",LF);
	    stream("</payload>",LF);
	}
    }

    private void produceData(Servicer servicer) throws ServletException {

	Connection connection = null;
	try {
	    connection = connMgr.acquire();
	    queryData(servicer,connection);
	} catch (DbConnectionMgrException e) {
	    stream("<quality error=\"1\" code=\"???\" message=\"" + e.getMessage() + "\"/>",LF);
	    stream("</payload>",LF);
	}finally {
	    try {
		connMgr.release(connection);
	    } catch (DbConnectionMgrException e) {}
	}
    }

    private void queryData(Servicer servicer, Connection connection) 
	throws ServletException {
	Statement stmt = null;
	String sql = servicer.getSql();
	try {
	    stmt = connection.createStatement();
	    ResultSet resultSet = stmt.executeQuery(sql);
	    marshal(servicer,resultSet);
	} catch (SQLException e) {
	    stream("<quality error=\"1\" code=\"???\" message=\"" + e.getMessage() + "\"/>",LF);
	    stream("</payload>",LF);
	} finally {
	    try {
		stmt.close();
	    } catch (SQLException e) {}
	}
    }

    private void marshal(Servicer servicer, ResultSet resultSet) 
	throws SQLException, ServletException {
	try {
	    stream("<data>",noLF);
	    long recordCnt = servicer.marshal(encoder,resultSet);
	    byte[] md5_digest=encoder.getMD5Digest();
	    StringBuffer md5_ascii=new StringBuffer("");
	    for(int i=0;i<md5_digest.length;i++)
	     {
	      int v=(int)md5_digest[i];
	      if(v<0) v=256+v;
	      String str=Integer.toString(v,16);
	      if(str.length()==1) md5_ascii.append("0");
	      md5_ascii.append(str);
	     }
	    stream("</data>",LF);
	    stream("<quality error=\"0\" md5=\""+md5_ascii+"\" records=\"" + recordCnt + "\"/>",LF);
	    stream("</payload>",LF);
	} catch (SQLException e) {
	    stream("</data>",LF);
	    throw e;
	} catch (Exception e) {
	    throw new ServletException(e);
	}
    }

    /**
     * Insures required keys common to all requests exist and are valid. As a 
     * side effect this class will set values internal to this class.
     *
     * @param  command  An instance of the Command class.
     * @return void
     * @except RequestHandlerExcepiton thrown if the command is invalid.
     *
     */
    private void validate(Command command) throws RequestHandlerException { 
	encoding = command.get("encoding");
	if (encoding == null)
	    throw new RequestHandlerException("Missing the tag 'encoding'");
	String type = command.get("type");
	if (type == null)
	    throw new RequestHandlerException("Missing the tag 'type'");
	else {
	    StringTokenizer typeComponets = new StringTokenizer(type,":");
	    if (typeComponets.countTokens() != 2)
		throw new RequestHandlerException("Malformed tag 'type', value of type: " + typeComponets);
	    else {
		objectName    = typeComponets.nextToken();
		objectVersion = typeComponets.nextToken();
	    }
	}
    }

    /**
     * Creates and returns an Encoder of the type set in a private class variable.  
     *
     * @except RequestHandlerExcepiton thrown if the command is invalid.
     *
     */
    private Encoder getEncoder(ServletOutputStream writer) throws RequestHandlerException {
	Encoder encoder = null;
	try {
	    if (encoding.compareToIgnoreCase("blob")==0)
		encoder = new BlobEncoder(writer);
	    else if (encoding.compareToIgnoreCase("xml")==0)
		throw new RequestHandlerException("XML encoding is not yet supported.");
	    else if (encoding.compareToIgnoreCase("csv")==0)
		throw new RequestHandlerException("CSV encoding is not yet supported.");
	    else
		throw new RequestHandlerException("Received unknown encoding type of " + encoding);
	} catch (Exception e) {
	    throw new RequestHandlerException(e.getMessage());
	}
	return encoder;
    }

    private void stream(String line,boolean lineFeed) throws ServletException {
	try {
	    if (lineFeed)
		writer.println(line);
	    else
		writer.print(line);

	} catch (IOException e) {
	    throw new ServletException(e.getMessage());
	}
    }

}
