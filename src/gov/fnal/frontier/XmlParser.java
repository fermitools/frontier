package gov.fnal.frontier;

import java.util.List;
import java.util.Iterator;
import org.jdom.Element;
/**
 * XML parser which supports version 1.0 of the XSD.  It is responsible for parsing the XSD into
 * a {@link XmlDescriptor}.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class XmlParser extends Parser {

    /**
    * Constructor.
    * @param type String naming the object which is to be parsed - "type" tag from descriptor.
    * @param version String identifying the version of the object - "version" tag from descriptor.
    * @param xsdVersion String identifying the XSD verion the XML data uses - "xsdversion" tag
    * from descriptor.
    */
    XmlParser(String type, String version, String xsdVersion) {
        super(type, version, xsdVersion);
    }

    /**
     * Parses the XML data into a {@link XmlDescriptor}.
     * @param root Element the top jdom Element which contains all other Elements.
     * @throws LoaderException
     * @return XmlDescriptor
     */
   public XmlDescriptor parse(Element root) throws LoaderException {
        List elements = root.getChildren();
        Iterator i = elements.iterator();
        String name = null;
        while(i.hasNext()) {
            Element element = (Element) i.next();
            name = element.getName();
            processElement(element);
        }
        descriptor.validate();
        return descriptor;
    }

    /**
     * Manages the parsing of a single element.
     * @param element Element
     * @throws LoaderException
     */
    private void processElement(Element element) throws LoaderException {
        String name = element.getName();
        if(name.equals("select"))
            descriptor.setSelectClause(element.getTextTrim());
        else if(name.equals("from"))
            descriptor.setFromClause(element.getTextTrim());
        else if(name.equals("final"))
            descriptor.setFinalClause(element.getTextTrim());
        else if(name.equals("attribute"))
            processAttribute(element);
        else if(name.equals("where"))
            processWhere(element);
        else
            throw new LoaderException("Unknown Element " + element.getName()
                                      + " in supplied in XSD.");
    }

    /**
     * Parses the data from an "attribute" tag.
     * @param element Element
     * @throws LoaderException
     */
    private void processAttribute(Element element) throws LoaderException {
        String type = element.getAttributeValue("type");
        String field = element.getAttributeValue("field");
        descriptor.addAttribute(type, field);
    }

    /**
     * Parses the data from a "where" tag and its sub-tags.
     * @param element Element
     * @throws LoaderException
     */
    private void processWhere(Element element) throws LoaderException {
        List children = element.getChildren();
        Iterator i = children.iterator();
        String name = null;
        String clause = "";
        String param = "";
        WhereClause where = descriptor.addWhereClause();
        while(i.hasNext()) {
            Element child = (Element) i.next();
            name = child.getName();
            if(name.equals("clause")) {
                if(clause.equals("")) {
                    clause = child.getTextTrim();
                    where.addClause(clause);
                } else
                    throw new LoaderException("where contains multiple 'clause' tags");
            } else if(name.equals("param"))
                where.addParam(child.getAttributeValue("type"), child.getAttributeValue("key"));
            else
                throw new LoaderException("where contains unknown element " + name);
        }
    }
}
