package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class ServicerException extends RequestHandlerException {

    /**
     * Constructor.
     */
    ServicerException() {}

    /**
     * Constructor
     * @param message String informational message about the exception.
     */
    ServicerException(String message) {
        super(message);
    }
}
