package gov.fnal.frontier;

import java.sql.Connection;
import gov.fnal.frontier.codec.Encoder;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class XmlServicer extends Servicer {

    XmlDescriptor descriptor = null;
    String sqlStatement = null;

    XmlServicer(XmlDescriptor aDescriptor) {
        descriptor = aDescriptor;
    }

    /**
     * Validates that the data in the Command produces a
     * vaild SQL where clause.  As a imporant side effect the SQL
     * select statement is built and stored within the instance. If the Commmad
     * is not valid an exception is thrown.
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

    public void process(Connection connection, Encoder encoder) throws ServicerException {

    }
}
