package gov.fnal.frontier;

import java.sql.Connection;
import gov.fnal.frontier.codec.Encoder;
import java.util.ListIterator;

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
     * @param command An instance of the Command class.
     * @exception ServicerValidationExeption Thrown if a valid
     * SQL statement cannot be created.
     * @return long.
     *
     */
    public void validateAndLoad(Command command) throws ServicerValidationException;

    /**
     * Controls querying the data from the data base and encoding that data onto the stream.
     * @param connection Connection live connection to the database.
     * @param encoder Encoder a sub-calls of Encoder to use for marshalling data to a stream.
     * @throws ServicerException
     * @return long the total number of records marshalled.
     */
    public long process(Connection connection, Encoder encoder) throws ServicerException;

    /**
     * Returns an iterator which when used will return an Object which may be cast to an
     * {@link Attribute} object.
     * @return ListIterator
     */
    public ListIterator getAttributes();

}
