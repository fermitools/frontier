package gov.fnal.frontier;

/**
 * Contains information intended to provide a unique identifier of the thread, and hopefully
 * someday of the client.
 * @version $Revision$
 * @author Stephen P. White <swhite@fnal.gov>
 */
public class Identifier {

    private long id;
    private String identifier;

    /**
     * Constructor.
     * @param aId long a unique identifier.
     */
    public Identifier(long aId) {
        id = aId;
        identifier = Long.toString(id);
    }

    /**
     * Return the identification string.
     * @return String
     */
    public String getIdentifier() {
        return identifier;
    }
}
