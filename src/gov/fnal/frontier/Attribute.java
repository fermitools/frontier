package gov.fnal.frontier;


/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class Attribute {

    private String type  = null;
    private String field = null;

    Attribute(String aType, String aField) throws LoaderException {
	if (aType.equals("")) 
	    throw new LoaderException("An attribute's type may not be null.");
	else if (aField.equals(""))
	    throw new LoaderException("An attribute's field may not be null.");
	type  = aType;
	field = aField;
    }

    public String getType() {
	return type;
    }

    public String getField() {
	return field;
    }
}
			     
