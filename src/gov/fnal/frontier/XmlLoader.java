package gov.fnal.frontier;

import java.io.InputStream;
import java.io.IOException;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

/**
 * This class will accepts a stream of data and produces a {@link XmlDescriptor} object.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class XmlLoader extends Loader {

    private class RootReturn {
        Element root = null;
        String type = null;
        String version = null;
        String xsdVersion = null;
        RootReturn(Element aRoot, String aType, String aVersion, String aXsdVersion) {
            root = aRoot;
            type = aType;
            version = aVersion;
            xsdVersion = aXsdVersion;
        }
    }

    /**
     * Constructor.
     */
    XmlLoader() {}

    /**
     * Converts a stream of XSD data into a {@link XmlDescriptor} object.
     * @param name String name of the object the data is for.  This method insures that the
     * provided name of the object version is the same as that described in the XSD. Were making
     * sure you got what you asked for.
     * @param aVersion String version of the object.  This method insures the given version of the
     * object is the same as described in the XSD.
     * @param data InputStream XSD data stream conforming to the Frontier specifications which
     * describe a table and how to query it.
     * @throws LoaderException for any XSD errors.
     * @return Descriptor returns a {@link XmlDescriptor} object as its parent class.
     */
    public Descriptor load(String name, String aVersion, InputStream data) throws LoaderException {
        RootReturn rootData = readXsdRoot(data);
        if (!rootData.type.equals(name)) {
            String msg = "The XSD's 'type' tag does not match type supplied on URL. ";
            msg += rootData.type + " != " + name;
            throw new LoaderException(msg);
        } else if (!rootData.version.equals(aVersion)) {
            String msg = "The XSD's 'xsdversion' tag does not match version supplied on URL. ";
            msg += rootData.version + " != " + aVersion;
            throw new LoaderException(msg);
        }
        Parser parser = getXsdVersion(rootData.type, rootData.version, rootData.xsdVersion);
        return parser.parse(rootData.root);
    }

    /**
     * Converts a stream of XSD data into a {@link XmlDescriptor} object.
     * @param data InputStream XSD data stream which conforms to the Frontier specificaitions.
     * @throws LoaderException for any syntax errors.
     * @return Descriptor
     */
    public Descriptor load(InputStream data) throws LoaderException {
        RootReturn rootData = readXsdRoot(data);
        Parser parser = getXsdVersion(rootData.type, rootData.version, rootData.xsdVersion);
        return parser.parse(rootData.root);
    }

    private RootReturn readXsdRoot(InputStream data) throws LoaderException {
        Element root = null;
        String type = null;
        String version = null;
        String xsdVersion = null;
        try {
            SAXBuilder builder = new SAXBuilder();
            Document doc = builder.build(data);
            root = doc.getRootElement();
            type = root.getAttributeValue("type");
            version = root.getAttributeValue("version");
            xsdVersion = root.getAttributeValue("xsdversion");
            if (type == null)
                throw new LoaderException("The XSD is missing the tag 'type' or it is null.");
            if (version == null)
                throw new LoaderException("The XSD is missing the tag 'version' or it is null.");
            if (xsdVersion == null)
                throw new LoaderException("The XSD is missing the tag 'xsdversion' or it is null.");
        } catch (Exception e) {
	    e.printStackTrace();
	    Frontier.Log("Error: "+e,e);
            throw new LoaderException(e.getMessage());
        }
        return new RootReturn(root, type, version, xsdVersion);
    }

    /**
     * Determines which version of the XmlParser to use and creates a instance of it.  Addition
     * versions of the XmlParser will be added to this method.
     * @param type String is the name of the object the XSD is to be parsed for.
     * @param version String is the specific version of the object the XSD is to be parsed for.
     * @param xsdVersion String is the version of the protocol in use for this XSD.
     * @throws LoaderException
     * @return Parser is an instance created to parse the specific protocol in use for this XSD.
     */
    private Parser getXsdVersion(String type, String version, String xsdVersion) throws
        LoaderException {
        Parser parser;
        if (xsdVersion.equals("1.0") || xsdVersion.equals("1"))
            parser = new XmlParser(type, version, xsdVersion);
        else
            throw new LoaderException("XSD version " + xsdVersion + " is not supported.");
        return parser;
    }

}
