package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class LoaderException extends RequestHandlerException {

    LoaderException() {}

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    LoaderException(String message) {
        super(message);
    }
}
