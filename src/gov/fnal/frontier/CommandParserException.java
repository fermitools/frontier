package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class CommandParserException extends Exception {

    /**
     * Constructor.
     */
    CommandParserException() {}

    /**
     * Constructor
     * @param message String informational data about the exception.
     */
    CommandParserException(String message) {
        super(message);
    }

}
