package gov.fnal.frontier;

import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * Controls the processing of adminstration requests. Well, it will when it is
 * filled out. ;-)
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class AdministrationRequestHandler extends RequestHandler {

    /**
     * Constructor
     * @param id {@link Identifier}
     * @param writer ServletOutputStream Stream to write results on.
     */
    AdministrationRequestHandler(ServletOutputStream writer) {
        super(writer);
    }

    /**
     * Recives a administration {@link Command}, processes it and writes it
     * results to the given stream.
     * @param command Command Instance to be processed.
     * @throws RequestHandlerException
     * @throws ServletException
     */
    public void process(Command command) throws RequestHandlerException, ServletException {
        throw new RequestHandlerException("Admin commands are not yet supported.");
    }
}
