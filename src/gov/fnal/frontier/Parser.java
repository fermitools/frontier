package gov.fnal.frontier;

import org.jdom.Element;

/**
 * Generic XML parser from which all versions are desended.  Its puporse is to allow different
 * versions of XSD parsers to be created and simitanously supported.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class Parser {

    protected XmlDescriptor descriptor = null;

    /**
     * Constructor.
     * @param type String naming the object which is to be parsed - "type" tag from descriptor.
     * @param version String identifying the version of the object - "version" tag from descriptor.
     * @param xsdVersion String identifying the XSD verion the XML data uses - "xsdversion" tag
     * from descriptor.
     */
    Parser(String type, String version, String xsdVersion) {
        descriptor = new XmlDescriptor(type, version, xsdVersion);
    }

    /**
     * Parses the XML data into a {@link XmlDescriptor} according to the version of XSD specified
     * in the constructor.
     * @param root Element the top jdom Element which contains all other Elements.
     * @throws LoaderException
     * @return XmlDescriptor
     */
    public XmlDescriptor parse(Element root) throws LoaderException {
        return descriptor;
    }

}
