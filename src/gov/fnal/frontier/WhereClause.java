package gov.fnal.frontier;

import java.util.ArrayList;

/**
 * This class describes a single SQL styled where statement and the parameter(s) it contains.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class WhereClause {

    ArrayList params = new ArrayList();
    String[] phrases = null;

    /**
     * Private class internal to WhereClause which identifies a single parameter of the
     * where statement.
     * @author Stephen P. White <swhite@fnal.gov>
     * @version $Revision$
     */
    private class Parameter {
        public String type = null;
        public String key = null;

        /**
         * Constructor.
         * @param aType String
         * @param aKey String
         */
        Parameter(String aType, String aKey) {
            type = aType;
            key = aKey;
        }
    }

    /**
     * Constructor.
     */
    WhereClause() {}

    /**
     * Accepts the data portion of a "clause" tag, parses it into its static and parameterized
     * data and saves this information.  The "clause" data must conform to the frontier
     * specifications.
     * @param clause String
     */
    public void addClause(String clause) {
        phrases = clause.split("@param");
    }

    /**
     * Saves the data which describes a single parameter of a "clause".
     * @param type String  a valid "type" as described by the Frontier documents.
     * @param key String the name of this parameter.
     * @throws LoaderException thrown if an unknown type is supplied.
     */
    public void addParam(String type, String key) throws LoaderException {
        if ( (type.compareToIgnoreCase("int")    == 0) ||
             (type.compareToIgnoreCase("long")   == 0) ||
             (type.compareToIgnoreCase("double") == 0) ||
             (type.compareToIgnoreCase("float")  == 0) ||
             (type.compareToIgnoreCase("string") == 0) ||
             (type.compareToIgnoreCase("date")   == 0) )
            params.add(new Parameter(type, key));
        else
            throw new LoaderException("WhereClause.addParam - received unknown type of " + type);
    }

    /**
     * Insures that all required data has been provided.
     * @throws LoaderException if data is missing.
     */
    public void validate() throws LoaderException {
        if(params.size() == 0) {
            if(phrases.length > 1)
                throw new LoaderException(
                    "Where clause - not enough keys were included for the @params in the clause.");
        } else if(params.size() != phrases.length)
            throw new LoaderException(
                "Where clause - not enough keys were included for the @params in the clause.");
    }

    /**
     * Returns the number of paramaters which exist for this instance.
     * @return int
     */
    public int getParamCount() {
        return params.size();
    }

    /**
     * Returns true if the suppled parameter exists.
     * @param aParam String name of the parameter to search for.
     * @return boolean
     */
    public boolean paramExists(String aParam) {
        for(int i = 0; i < params.size(); i++) {
            if(aParam.equals( ( (Parameter) params.get(i)).key))
                return true;
        }
        return false;
    }

    /**
     * Constructs a where clause, replacing all the paramaters with their approprite values.
     * @param command Command instance containing the key/values which are to be put in the where
     * clause.
     * @return String containing a parameter substituted where clause.
     */
    public String buildWhere(Command command) {
        String where = "";
        for(int i=0;i<phrases.length;i++) {
            where += phrases[i];
            if (i < params.size()) {
                String key = ( ( (Parameter) params.get(i)).key);
                String value = command.get(key);
                where += value + " ";
            }
        }
        return where;
    }

    /**
     * Dumps out a printable version of the instance.
     * @return String
     */
    public String dump() {
        String where = "";
        for(int i=0;i<phrases.length;i++) {
            where += phrases[i];
            if (i < params.size()) {
                String key = ( ( (Parameter) params.get(i)).key);
                where += "@" + key + " ";
            }
        }
        return where;
    }
}
