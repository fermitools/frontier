package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class LoaderException extends RequestHandlerException {

    public LoaderException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    public LoaderException(String message) {
        super(message);
    }
}
