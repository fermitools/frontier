package gov.fnal.frontier;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class XmlServicer extends Servicer {

    XmlDescriptor descriptor = null;
    String whereClause       = null;

    XmlServicer(XmlDescriptor aDescriptor) {
	descriptor = aDescriptor;
    }

    public void validate(Command command) throws ServicerValidationException {

	whereClause = descriptor.findAndBuildWhereClause(command);
	if (whereClause == null)
	    throw new ServicerValidationException("Unable to find a where clause which matches the supplied keys.");
	throw new ServicerValidationException("Your where clause Sir!  where:: " + whereClause);
    }    
}
