package gov.fnal.frontier;


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

    XmlDescriptor(String aName, String aVersion, String aXsdVersion) {
	name       = aName;
	version    = aVersion;
	xsdVersion = aXsdVersion;
    }

    public void setSelectClause(String aSelectClause) {
	selectClause = aSelectClause;
    }

    public void setFromClause(String aFromClause) {
	fromClause = aFromClause;
    }

    public void setFinalClause(String aFinalClause) {
	finalClause = aFinalClause;
    }

}

