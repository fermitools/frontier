package gov.fnal.frontier;

import gov.fnal.frontier.codec.*;
import java.util.*;
import java.io.*;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class UniversalQueryRequestHandler extends RequestHandler {

    private String  encoding      = null;
    private String  objectName    = null;
    private String  objectVersion = null;
    private Encoder encoder       = null;


    UniversalQueryRequestHandler() {}
    
    public void process(Command command, ServletOutputStream writer) 
	throws RequestHandlerException, ServletException {

	validate(command);
	encoder = getEncoder(writer);

	Servicer servicer = new Servicer();
	try {
	    servicer.validate(Command,command);
	    stream("<payload type=\"" + objectName + " version=\"" + objectVersion + " encoding=\"" + encoding + "\">");
	    stream("<data>");
	
	    stream("data data data");
	    
	    stream("</data>");
	    stream("<quality stuff here/>");
	    stream("</payload>");
	} catch (ServicerValidationException e) {
	    stream("<quality error=\"1\: code=\"???\" message=\"" + e.getMessage() + "\"/>");
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

    void stream(String line) throws ServletException {
	try {
	    writer.println(line);

	} catch (IOException e) {
	    throw new ServletException(e.getMessage());
	}
    }

}
