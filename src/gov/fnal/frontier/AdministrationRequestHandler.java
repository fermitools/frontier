package gov.fnal.frontier;

import java.io.IOException;
import java.util.*;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * Controls the processing of adminstration requests.
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class AdministrationRequestHandler extends RequestHandler {



    AdministrationRequestHandler(ServletOutputStream writer) {
	super(writer);
    }
    
    public void process(Command command)
	throws RequestHandlerException, ServletException {

	throw new RequestHandlerException("Admin commands are not yet supported.");
    }

}