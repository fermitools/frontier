package gov.fnal.frontier;

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class CommandException extends Exception {

    /**
     * Constructor
     */
    CommandException() {}

    /**
     * Constructor.
     * @param message String informational data about the exception
     */
    CommandException(String message) {
        super(message);
    }

}
