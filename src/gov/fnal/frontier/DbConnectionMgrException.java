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
class DbConnectionMgrException extends Exception {
    public DbConnectionMgrException() {}

    /**
     * Constructor
     * @param message String informational data about the exception.
     */
    public DbConnectionMgrException(String message) {
        super(message);
    }
}
