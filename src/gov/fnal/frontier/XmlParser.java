package gov.fnal.frontier;

import java.util.List;
import java.util.Iterator;
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

public class XmlParser extends Parser {

    XmlParser(String type, String version, String xsdVersion) {
	super(type,version,xsdVersion);
    }


    public XmlDescriptor parse(Element root) throws LoaderException {
	List elements = root.getChildren();
	Iterator i = elements.iterator();
	String name = null;
	while (i.hasNext()) {
	    Element element = (Element) i.next();
	    name = element.getName();
	    processElement(element);
	}
	descriptor.validate();
	return descriptor;
    }
    
    private void processElement(Element element) throws LoaderException {
	String name = element.getName();
	if (name.equals("select"))
	    descriptor.setSelectClause(element.getTextTrim());
	else if (name.equals("from"))
	    descriptor.setFromClause(element.getTextTrim());
	else if (name.equals("final"))
	    descriptor.setFinalClause(element.getTextTrim());
	else if (name.equals("attribute"))
	    processAttribute(element);
	else if (name.equals("where"))
	    processWhere(element);
	else
	    throw new LoaderException("Unknown Element " + element.getName() + " in supplied in XSD.");
    }

    private void processAttribute(Element element) throws LoaderException {
	String type  = element.getAttributeValue("type");
	String field = element.getAttributeValue("field");
	descriptor.addAttribute(type,field);
    }

    private void processWhere(Element element) throws LoaderException {
	List children = element.getChildren();
	Iterator i = children.iterator();
	String name = null;
	String clause = "";
	String param  = "";
 	WhereClause where = descriptor.addWhereClause();
	while (i.hasNext()) {
	    Element child = (Element) i.next();
	    name = child.getName();
	    if (name.equals("clause")) {
		if (clause.equals("")) {
		    clause = child.getTextTrim();
		    where.addClause(clause);
		}
		else
		    throw new LoaderException("where contains multiple 'clause' tags");
	    }
	    else if (name.equals("param"))
		where.addParam(child.getAttributeValue("type"),child.getAttributeValue("key"));
	    else
		throw new LoaderException("where contains unknown element " + name);
	}
    }
}

