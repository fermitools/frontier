package gov.fnal.frontier;

import java.io.*;
import org.jdom.*;
import org.jdom.input.*;
import org.jdom.output.*;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class XmlLoader extends Loader {

    XmlLoader() {}

    public Descriptor load(String name, String aVersion, InputStream data) 
	throws LoaderException {
	Descriptor   descriptor = null;
	try {
            // Build the document with SAX and Xerces, no validation
            SAXBuilder builder = new SAXBuilder();
            // Create the document
            Document doc = builder.build(data);


	    Element root      = doc.getRootElement();
	    String type       = root.getAttributeValue("type");
	    String version    = root.getAttributeValue("version");
	    String xsdVersion = root.getAttributeValue("xmlversion");
	    Parser parser     = null;
	    if (! type.equals(name)) {
		String msg = "XSD type tag does not match type supplied on URL. ";
		msg += type + " != " + name;
		throw new LoaderException(msg);
	    }
	    else if (! version.equals(aVersion)) {
		String msg = "XSD version tag does not match version supplied on URL. ";
		msg += version + " != " + aVersion;
		throw new LoaderException(msg);
	    }
	    else if (xsdVersion.equals("1.0"))
		parser = new XmlParser(type,version,xsdVersion);
	    else 
		throw new LoaderException("XSD version " + xsdVersion + " is not supported.");
	    descriptor = parser.parse(root);
	} catch (IOException e) {
	    throw new LoaderException(e.getMessage());
	} catch (JDOMException e) {
	    throw new LoaderException(e.getMessage());
	}

	return descriptor;
    }
}
