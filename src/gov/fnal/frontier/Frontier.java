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

    private DbConnectionMgr connMgr     = null;
    private CommandParser   parser      = null;
    private ArrayList       commandList = null;

    public void init() throws ServletException {
	connMgr = DbConnectionMgr.getDbConnectionMgr();
	parser  = new CommandParser();
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

        response.setContentType("text/plain");
        PrintWriter writer = response.getWriter();
	String queryString = request.getQueryString();

	try {
	    commandList = parser.parse(queryString);
	} catch (CommandParserException e) {
	    throw new ServletException(e.getMessage());
	}

        writer.println("----------------------------------------");
	writer.println("Ok, I'm up and running....");
	writer.println("queryString: " + queryString);
        writer.println("----------------------------------------");
	writer.flush();
    }

} // class Frontier
