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

    public Descriptor load(InputStream data) 
	throws LoaderException {
	DocumentBuilder builder = null;
	Document doc = null;
	try {
	    DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
	    builder = factory.newDocumentBuilder();
	    doc = builder.parse(data);


	    Element root = doc.getDocumentElement();
	    System.out.println("Root Name: " + root.getTagName());
	    System.out.println("type: " + root.getAttribute("type"));
	    
	    NodeList children = root.getChildNodes();
	    for (int i=0;i<children.getLength();i++) {
		Node child = children.item(i);
		if (child instanceof Element) {
		    Element childElement = (Element) child;
		    System.out.println("Element name: " + childElement.getTagName());
		}
	    }


	} catch (IOException e) {
	    System.out.println("1 - error: " + e.getMessage());
	    throw new LoaderException(e.getMessage());
	} catch (ParserConfigurationException e) {
	    System.out.println("2 - error: " + e.getMessage());
	    throw new LoaderException(e.getMessage());
	} catch (SAXException e) {
	    System.out.println("3 - error: " + e.getMessage());
	    System.out.println(data.toString());
	    throw new LoaderException(e.getMessage());
	}

	return null;
    }
}
