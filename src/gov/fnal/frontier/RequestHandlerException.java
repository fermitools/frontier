package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */

public class RequestHandlerException extends Exception {

    /**
     * Constructor.
     */
    RequestHandlerException() {}

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    RequestHandlerException(String message) {
        super(message);
    }

}
