package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class ServicerFactoryException extends FrontierException {

    /**
     * Constructor.
     */
    public ServicerFactoryException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational message about the exception.
     */
    public ServicerFactoryException(String message) {
        super(message);
    }

}
