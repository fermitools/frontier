package gov.fnal.frontier;

import java.sql.Connection;
import gov.fnal.frontier.codec.Encoder;
import java.sql.Connection;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ListIterator;

/**
 * This class creates uses data from an {@link XmlDescriptor} to produce obtain data from
 * the data source and to stream that data out.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */

public class XmlServicer implements Servicer {

    XmlDescriptor descriptor = null;
    String sqlStatement = null;

    /**
     * Constructor.
     * @param aDescriptor XmlDescriptor instance of the descriptor to tie the instance of
     * this class.
     */
    XmlServicer(XmlDescriptor aDescriptor) {
        descriptor = aDescriptor;
    }

    /**
     * Validates that the data in the Command produces a valid SQL where clause.  As a imporant side
     * effect the SQL select statement is built and stored within the instance. If the Commmad does
     * not produce valid SQL then an exception is thrown.
     * @param command An instance of the {@link Command} class.
     * @exception ServicerValidationExeption Thrown if unable to locate a valid where clause.
     * SQL statement cannot be created.
     * @return void.
     */
    public void validateAndLoad(Command command) throws ServicerValidationException {

        String whereClause = descriptor.findAndBuildWhereClause(command);
        if(whereClause == null)
            throw new ServicerValidationException(
                "Unable to find a where clause which matches the supplied keys.");
        sqlStatement = "SELECT " + descriptor.getSelectClause();
        sqlStatement += " FROM " + descriptor.getFromClause();
        sqlStatement += " WHERE " + whereClause;
        sqlStatement += " " + descriptor.getFinalClause();
    }

    /**
     * Controls querying the data from the database and encoding that data onto the stream.
     * @param connection Connection live connection to the database.
     * @param encoder Encoder a sub-calls of Encoder to use for marshalling data to a stream.
     * @throws ServicerException
     * @return long the total number of records marshalled.
     */
    public long process(Connection connection, Encoder encoder) throws ServicerException {
        long recordCnt = 0;
        Statement stmt = null;
        ResultSet rs = null;
        try {
            stmt = connection.createStatement();
            rs = stmt.executeQuery(sqlStatement);
            while(rs.next()) {
                recordCnt++;
                write(encoder, rs);
                encoder.writeEOR();
            }
            encoder.flush();
        } catch(SQLException e) {
            throw new ServicerException(e.getMessage());
        } catch(Exception e) {
            throw new ServicerException(e.getMessage());
        } finally {
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {}
            try {
                if (stmt != null)
                    stmt.close();
            } catch (SQLException e) {}
        }
        return recordCnt;
    }

    /**
     * Returns an iterator which when used will return an Object which may be cast to an
     * {@link Attribute} object.
     * @return ListIterator
     */
    public ListIterator getAttributes() {
        return descriptor.getAttributes();
    }

    /**
     * Determines the type of data each field must be converted to and marshalls the data.
     * @param encoder Encoder a sub-calls of Encoder to use for marshalling data to a stream.
     * @param rs ResultSet a database row whose columns are to be marshalled.
     * @throws ServicerException
     */
    private void write(Encoder encoder, ResultSet rs) throws ServicerException {
        for(int columnCnt = 1; columnCnt < descriptor.getAttributeCount() + 1; ++columnCnt)
            try {
                String columnType = descriptor.getAttributeType(columnCnt - 1);
                //String field = descriptor.getAttributeField(columnCnt - 1);
                //System.out.println("field: " + field + "  columnType: " + columnType);
                if(columnType.compareToIgnoreCase("int") == 0)
                    encoder.writeInt(rs.getInt(columnCnt));
                else if(columnType.compareToIgnoreCase("long") == 0)
                    encoder.writeLong(rs.getLong(columnCnt));
                else if(columnType.compareToIgnoreCase("double") == 0)
                    encoder.writeDouble(rs.getDouble(columnCnt));
                else if(columnType.compareToIgnoreCase("float") == 0)
                    encoder.writeFloat(rs.getFloat(columnCnt));
                else if(columnType.compareToIgnoreCase("string") == 0)
                    encoder.writeString(rs.getString(columnCnt));
                else if(columnType.compareToIgnoreCase("bytes") == 0)
                    encoder.writeBytes(rs.getBytes(columnCnt));
                else if(columnType.compareToIgnoreCase("date") == 0)
                    encoder.writeDate(rs.getDate(columnCnt));
                else
                    throw new ServicerException("xmlServicer.process - unknown attribute - "
                                                + descriptor.getAttributeType(columnCnt - 1));
            } catch(Exception e) {
                throw new ServicerException("XmlServicer.write - " + e.getMessage());
            }
    }

}
