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
    public CommandParserValueException() {
        super();
    }

    /**
     * Constructor.
     * @param message String informational data about the exception.
     */
    public CommandParserValueException(String message) {
        super(message);
    }

}
