package gov.fnal.frontier;

/**
 * Stores data describing a single field of the database.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class Attribute {

    private String type = null;
    private String field = null;

    /**
     * Constructor.
     * @param aType String identifing the a valid Frontier type for the field.
     * @param aField String naming the field.
     * @throws LoaderException if any field is null.
     */
    Attribute(String aType, String aField) throws LoaderException {
        if(aType.equals(""))
            throw new LoaderException("An attribute's type may not be null.");
        else if(aField.equals(""))
            throw new LoaderException("An attribute's field may not be null.");
        type = aType;
        field = aField;
    }

    /**
     * Returns the type of the attribute.
     * @return String
     */
    public String getType() {
        return type;
    }

    /**
     * Returns the name of the field.
     * @return String
     */
    public String getField() {
        return field;
    }

    /**
     * Insures the data conforms to Frontier requirements.
     * @throws LoaderException thrown if the type supplied in the constructor is unknown.
     */
    public void validate() throws LoaderException {
        if ( (type.compareToIgnoreCase("int")    != 0) &&
             (type.compareToIgnoreCase("long")   != 0) &&
             (type.compareToIgnoreCase("double") != 0) &&
             (type.compareToIgnoreCase("float")  != 0) &&
             (type.compareToIgnoreCase("string") != 0) &&
             (type.compareToIgnoreCase("bytes")  != 0) &&
             (type.compareToIgnoreCase("date")   != 0) )
            throw new LoaderException("Attribute.validate - received unknown type of "
                                      + type + "for the field " + field);

    }
}
