package gov.fnal.frontier;

/**
 * Base Exception for DbConectionMgr.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */

/**
 * Exception
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
class DbConnectionMgrException extends FrontierException {
    public DbConnectionMgrException() {
        super();
    }

    /**
     * Constructor
     * @param message String informational data about the exception.
     */
    public DbConnectionMgrException(String message) {
        super(message);
    }
}
