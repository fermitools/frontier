package gov.fnal.frontier;

import java.util.*;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class RequestHandler {

    ServletOutputStream writer = null;

    RequestHandler(ServletOutputStream aWriter) {
	writer = aWriter;
    }
    
    public void process(Command command)
	throws RequestHandlerException, ServletException {

    }

}

    
