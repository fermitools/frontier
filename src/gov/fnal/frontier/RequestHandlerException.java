package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */

public class RequestHandlerException extends FrontierException {

    /**
     * Constructor.
     */
    public RequestHandlerException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    public RequestHandlerException(String message) {
        super(message);
    }

}
