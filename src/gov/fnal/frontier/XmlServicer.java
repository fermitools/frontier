package gov.fnal.frontier;

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
     * vaild SQL where clause.  As a side effect the SQL
     * select statement is built and loaded into the instance.
     *
     * @param command An instance of the Command class.
     * @exception ServicerValidationExeption Thrown if a valid
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
        throw new ServicerValidationException("SQL Statement: " + sqlStatement);
    }
}
