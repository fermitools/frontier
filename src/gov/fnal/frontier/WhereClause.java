package gov.fnal.frontier;

import java.util.ArrayList;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class WhereClause {

    ArrayList params = new ArrayList();
    String[] phrases = null;
    private class Parameter {
        public String type = null;
        public String key = null;

        Parameter(String aType, String aKey) {
            type = aType;
            key = aKey;
        }
    }

    WhereClause() {}

    public void addClause(String clause) {
        phrases = clause.split("@param");
        System.out.println("**phrases...");
        for(int i = 0; i < phrases.length; i++)
            System.out.println("phrase: " + phrases[i]);
    }

    public void addParam(String type, String key) {
        params.add(new Parameter(type, key));
    }

    public void validate() throws LoaderException {
        if(params.size() == 0) {
            //With no params where clauses allowed are NONE and
            //a single line which does not include @param.
            if(phrases.length > 1)
                throw new LoaderException(
                    "Where clause - not enough keys were included for the @params in the clause.");
        } else if(params.size() != phrases.length)
            throw new LoaderException(
                "Where clause - not enough keys were included for the @params in the clause.");
    }

    public int getParamCount() {
        return params.size();
    }

    public boolean paramExists(String aParam) {
        for(int i = 0; i < params.size(); i++) {
            if(aParam.equals( ( (Parameter) params.get(i)).key))
                return true;
        }
        return false;
    }

    String buildWhere(Command command) {
        String where = "";
        where += phrases[0];
        for(int i = 0; i < params.size(); i++) {
            String key = ( ( (Parameter) params.get(i)).key);
            String value = command.get(key);
            where += value + " ";
            if(i < phrases.length - 1)
                where += phrases[i];
        }
        return where;
    }
}
