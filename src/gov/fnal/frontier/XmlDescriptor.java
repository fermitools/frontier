package gov.fnal.frontier;

import java.util.ArrayList;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class XmlDescriptor extends Descriptor {

    private String name         = null;
    private String version      = null;
    private String xsdVersion   = null;
    private String selectClause = null;
    private String fromClause   = null;
    private String finalClause  = null;
    private ArrayList attributes   = new ArrayList();
    private ArrayList wheres       = new ArrayList();

    XmlDescriptor(String aName, String aVersion, String aXsdVersion) {
	name       = aName;
	version    = aVersion;
	xsdVersion = aXsdVersion;
    }

    public void validate() throws LoaderException {
    }

    public void setSelectClause(String aSelectClause) {
	selectClause = aSelectClause;
    }

    public String getSelectClause() {
	return selectClause;
    }

    public void setFromClause(String aFromClause) {
	fromClause = aFromClause;
    }

    public String getFromClause() {
	return fromClause;
    }

    public void setFinalClause(String aFinalClause) {
	finalClause = aFinalClause;
    }

    public String getFinalClause() {
	return finalClause;
    }

    public void addAttribute(String type, String field) throws LoaderException {
	attributes.add(new Attribute(type,field));
    }

    public WhereClause addWhereClause() {
	WhereClause where = new WhereClause();
	wheres.add(where);
	return where;
    }

}

