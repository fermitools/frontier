package gov.fnal.frontier;

import java.sql.Connection;
import gov.fnal.frontier.codec.Encoder;

/**
 * Generic interface whose methods must be overridden.  Its purpose is to read data
 * from the database and write it to the stream in the manner specified by
 * its {@link Descriptor}.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public interface Servicer {

    /**
     * Validates that the data in the Command produces a
     * vaild SQL where clause.  As a side effect the SQL
     * select statement is built and loaded into the instance.
     *
     * @param command An instance of the Command class.
     * @exception ServicerValidationExeption Thrown if a valid
     * SQL statement cannot be created.
     * @return long.
     *
     */
    public void validateAndLoad(Command command) throws ServicerValidationException;

    public long process(Connection connection, Encoder encoder) throws ServicerException;

}
