package gov.fnal.frontier;

import java.io.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.*;

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

