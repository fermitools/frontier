package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class CommandParserValueException extends CommandParserException {

    /**
     * Constructor.
     */
    CommandParserValueException() {}

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    CommandParserValueException(String message) {
        super(message);
    }

}
