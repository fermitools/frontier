package gov.fnal.frontier;

import java.io.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;
import com.oreilly.servlet.*;

/**
 * Top level frontier servlet object.
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */


public final class Frontier extends CacheHttpServlet {

    private String frontierVersion                    = "1.0";
    private String xmlVersion                         = "1.0";

    private DbConnectionMgr connMgr                   = null;
    private CommandParser   parser                    = null;
    private ArrayList       commandList               = null;
    private UniversalQueryRequestHandler queryHandler = null;
    private AdministrationRequestHandler adminHandler = null;
    private ServletOutputStream writer                = null;

    public void init() throws ServletException {
	connMgr      = DbConnectionMgr.getDbConnectionMgr();
	parser       = new CommandParser();
    }

    /**
     * Respond to a GET request for the content produced by this
     * servlet.  This function handles call-outs to the different
     * interfaces supported by this servlet.
     *
     * @param request  The servlet request we are processing
     * @param response The servlet response we are producing
     *
     * @exception IOException if an input/output error occurs
     * @exception ServletException if a servlet error occurs
     */
    public void doGet(HttpServletRequest request,
                      HttpServletResponse response)
        throws IOException, ServletException {

	String queryString = request.getQueryString();
        response.setContentType("text/plain");
	response.setCharacterEncoding("US-ASCII");
        writer = response.getOutputStream();

	stream("<?xml version=\"1.0\" encoding=\"US-ASCII\"?>");
	stream("<frontier version=\"" + frontierVersion + "\" xmlVersion=\"" + xmlVersion + "\">");
	try {
	    commandList = parser.parse(queryString);
	    connMgr.initialize();
	    handleRequest(commandList,writer);
	} catch (CommandParserException e) {
	    stream("<transaction payloads=\"0\">");
	    generateBadQualityTag(e.getMessage());
	} catch (DbConnectionMgrException e) {
	    System.out.println("Unable to obtain connection: " + e.getMessage());
	    generateBadQualityTag(e.getMessage());
	}
	stream("</frontier>");

	writer.close();
    }

    void handleRequest(ArrayList commandList, ServletOutputStream writer) throws ServletException {
	RequestHandler handler = null;
	int numCommands = commandList.size();
	stream("<transaction payloads=\"" + numCommands + "\">");
	for (int i=0;i<numCommands;i++) {
	    Command command = (Command) commandList.get(i);
	    if (command.isUniversalQueryCommand())
		handler = new UniversalQueryRequestHandler();
	    else if (command.isAdminCommand()) 
		handler = new AdministrationRequestHandler();
	    else
		throw new ServletException("Internal error, unknown type of Command.");
	    try {
		handler.process(command,writer);
	    } catch (RequestHandlerException e) {
		generateBadQualityTag(e.getMessage());
	    }
	}
	stream("</transaction>");
    }

    void stream(String line) throws ServletException {
	try {
	    writer.println(line);

	} catch (IOException e) {
	    throw new ServletException(e.getMessage());
	}
    }

    void generateBadQualityTag(String message) throws ServletException {
	stream("<quality error=\"1\" code=\"???\" message=\"" + message + "\"/>");
    }


} // class Frontier
