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
    public ServicerException() {
        super();
    }

    /**
     * Constructor
     * @param message String informational message about the exception.
     */
    public ServicerException(String message) {
        super(message);
    }
}
