package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class ServicerFactoryException extends Exception {

    /**
     * Constructor.
     */
    ServicerFactoryException() {}

    /**
     * Constructor.
     * @param message String informational message about the exception.
     */
    ServicerFactoryException(String message) {
        super(message);
    }

}
