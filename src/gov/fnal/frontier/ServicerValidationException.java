package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class ServicerValidationException extends ServicerException {

    /**
     * Constructor.
     */
    public ServicerValidationException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational message about the exception.
     */
    public ServicerValidationException(String message) {
        super(message);
    }
}
