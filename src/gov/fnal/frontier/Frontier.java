package gov.fnal.frontier;

import java.io.*;
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

    private DbConnectionMgr connMgr = null;

    public void init() throws ServletException {
	connMgr = DbConnectionMgr.getDbConnectionMgr();
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

        PrintWriter writer = response.getWriter();
	writer.println("Let's see if this thing works!!");
	writer.flush();
    }

} // class Frontier
