package gov.fnal.frontier;

import java.sql.Connection;
import gov.fnal.frontier.codec.Encoder;
import java.sql.Connection;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.Blob;
import java.sql.SQLException;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class XmlServicer implements Servicer {

    XmlDescriptor descriptor = null;
    String sqlStatement = null;

    XmlServicer(XmlDescriptor aDescriptor) {
        descriptor = aDescriptor;
    }

    /**
     * Validates that the data in the Command produces availd SQL where clause.  As a imporant side
     * effect the SQL select statement is built and stored within the instance. If the Commmad is
     * not valid an exception is thrown.
     * @param command An instance of the Command class.
     * @exception ServicerValidationExeption Thrown if unable to locate a valid where clause.
     * SQL statement cannot be created.
     * @return void.
     *
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

    public long process(Connection connection, Encoder encoder) throws ServicerException {
        long recordCnt = 0;
        try {
            Statement stmt = connection.createStatement();
            ResultSet rs = stmt.executeQuery(sqlStatement);
            while(rs.next()) {
                recordCnt++;
                for(int columnCnt=1;columnCnt<descriptor.getAttributeCount()+1;++columnCnt){
                    write(encoder, rs, columnCnt);
                }
            }
            stmt.close();
        } catch(SQLException e) {
            throw new ServicerException(e.getMessage());
       }
        return recordCnt;
    }

    private void write(Encoder encoder, ResultSet rs, int columnCnt) throws ServicerException {
        try {
            String columnType = descriptor.getAttributeType(columnCnt);
            System.out.println("columnType: " + columnType + "  columnCnt: " + columnCnt);
            if (columnType.compareToIgnoreCase("int")==0)
                encoder.writeInt(rs.getInt(columnCnt));
            else if (columnType.compareToIgnoreCase("long")==0)
                encoder.writeLong(rs.getLong(columnCnt));
            else if (columnType.compareToIgnoreCase("double")==0)
                encoder.writeDouble(rs.getDouble(columnCnt));
            else if (columnType.compareToIgnoreCase("float")==0)
                encoder.writeFloat(rs.getFloat(columnCnt));
            else if (columnType.compareToIgnoreCase("string")==0)
                    encoder.writeString(rs.getString(columnCnt));
            else if (columnType.compareToIgnoreCase("bytes")==0)
                encoder.writeBytes(rs.getBytes(columnCnt));
            else if (columnType.compareToIgnoreCase("date")==0)
                encoder.writeDate(rs.getDate(columnCnt));
            else
                throw new ServicerException("xmlServicer.process - unknown attribute - "
                                            + descriptor.getAttributeType(columnCnt));
        } catch(Exception e) {
            throw new ServicerException("XmlServicer.write - " + e.getMessage());
        }
    }

}
