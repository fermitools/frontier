package gov.fnal.frontier;

import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

/**
 * Base clase responsible for processing requests.  Each type of request
 * descends from this class and is expected to override its methods.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class RequestHandler {

    ServletOutputStream writer = null;

    /**
     * Constructor
     * @param aWriter ServletOutputStream Stream to which all data will be
     * output.
     */
    RequestHandler(ServletOutputStream aWriter) {
        writer = aWriter;
    }

    /**
     * Controls the processing of a single {@link Command} and writing of data
     * to the given stream.
     * @param command Command An instance of Command to be processed
     * @throws RequestHandlerException
     * @throws ServletException
     */
    public void process(Command command) throws RequestHandlerException, ServletException {

    }
}
