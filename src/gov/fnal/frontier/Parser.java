package gov.fnal.frontier;

import org.jdom.Element;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class Parser {

    protected XmlDescriptor descriptor = null;

    Parser(String type, String version, String xsdVersion) {
        descriptor = new XmlDescriptor(type, version, xsdVersion);
    }

    public XmlDescriptor parse(Element root) throws LoaderException {

        return descriptor;
    }

}
