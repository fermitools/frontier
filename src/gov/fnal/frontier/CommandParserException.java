package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class CommandParserException extends FrontierException {

    /**
     * Constructor.
     */
    public CommandParserException() {
        super();
    }

    /**
     * Constructor
     * @param message String informational data about the exception.
     */
    public CommandParserException(String message) {
        super(message);
    }

}
