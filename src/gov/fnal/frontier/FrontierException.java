package gov.fnal.frontier;



/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class FrontierException extends Exception {

    /**
     * Constructor
     */
    public FrontierException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational data about the exception
     */
    public FrontierException(String message) {
        super(message);
    }

}
