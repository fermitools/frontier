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

public class XmlParser extends Parser {

    XmlParser(String type, String version, String xsdVersion) {
	super(type,version,xsdVersion);
    }


    public XmlDescriptor parse(Element root) throws LoaderException {

	
	NodeList children = root.getChildNodes();
	for (int i=0;i<children.getLength();i++) {
	    Node child = children.item(i);
	    if (child instanceof Element) {
		Element childElement = (Element) child;
		processElement(childElement);
	    }
	}
	return descriptor;
    }
    
    private void processElement(Element element) throws LoaderException {
	String tagName = element.getTagName();
	if (tagName.equals("select"))
	    processSelect(element);
	else if (tagName.equals("from"))
	    processFrom(element);
	else if (tagName.equals("final"))
	    processFinal(element);
	else if (tagName.equals("attribute"))
	    processAttribute(element);
	else if (tagName.equals("where"))
	    processWhere(element);
	else
	    throw new LoaderException("Unknown Element " + element.getTagName() + " supplied.");

	System.out.println("****Element name: " + tagName);
    }

    private void processSelect(Element element) throws LoaderException {
	System.out.println("***************************");
	System.out.println("select: " + element.getNodeValue());
	System.out.println("***************************");
    }

    private void processFrom(Element element) throws LoaderException {
    }

    private void processFinal(Element element) throws LoaderException {
    }

    private void processAttribute(Element element) throws LoaderException {
    }

    private void processWhere(Element element) throws LoaderException {
    }

}

