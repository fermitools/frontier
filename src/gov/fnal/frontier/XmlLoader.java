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

public class XmlLoader extends Loader {

    XmlLoader() {}

    public Descriptor load(String name, String aVersion, InputStream data) 
	throws LoaderException {
	DocumentBuilder builder = null;
	Document            doc = null;
	Descriptor   descriptor = null;
	try {
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	    builder = factory.newDocumentBuilder();
	    doc = builder.parse(data);
	    Element root      = doc.getDocumentElement();
	    String type       = root.getAttribute("type");
	    String version    = root.getAttribute("version");
	    String xsdVersion = root.getAttribute("xmlversion");
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
	} catch (ParserConfigurationException e) {
	    throw new LoaderException(e.getMessage());
	} catch (SAXException e) {
	    throw new LoaderException(e.getMessage());
	}

	return descriptor;
    }
}
