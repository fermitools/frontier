package gov.fnal.frontier;

/**
 * Base Exception for DbConectionMgr.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
class DbConnectionMgrException extends Exception {
    public DbConnectionMgrException() {}

    public DbConnectionMgrException(String message) {
        super(message);
    }
}
