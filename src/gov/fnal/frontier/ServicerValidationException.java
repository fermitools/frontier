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
    ServicerValidationException() {}

    /**
     * Constructor.
     * @param message String informational message about the exception.
     */
    ServicerValidationException(String message) {
        super(message);
    }
}
