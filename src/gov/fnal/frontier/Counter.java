
package gov.fnal.frontier;

import java.io.PrintWriter;

import org.xml.sax.Attributes;
//import org.xml.sax.Parser;
import org.xml.sax.SAXException;
import org.xml.sax.SAXNotRecognizedException;
import org.xml.sax.SAXNotSupportedException;
import org.xml.sax.SAXParseException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;
import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.helpers.ParserAdapter;
//import org.xml.sax.helpers.ParserFactory;

import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import java.io.*;


  class MyResolver implements EntityResolver 
   {
   
    static final String dtd="<!ELEMENT descriptor (attribute+,select,from,where*,final)>"
+"<!ATTLIST descriptor "
+" type CDATA #REQUIRED "
+" version CDATA #REQUIRED "
+" xsdversion CDATA #REQUIRED> "
+"<!ELEMENT attribute EMPTY> "
+"<!ATTLIST attribute "
+" position CDATA #REQUIRED "
+" type (int|long|double|string) #REQUIRED "
+" field CDATA #REQUIRED> "
+"<!ELEMENT select (#PCDATA)> "
+"<!ELEMENT from (#PCDATA)> "
+"<!ELEMENT where (clause,param*)> "
+"<!ELEMENT clause (#PCDATA)> "
+"<!ELEMENT param EMPTY> "
+"<!ATTLIST param "
+" position CDATA #REQUIRED "
+" type (int|long|double|string) #REQUIRED "
+" key CDATA #REQUIRED> "
+"<!ELEMENT final (#PCDATA)>";

   
    public InputSource resolveEntity (String publicId, String systemId)
    {
     System.out.println("["+publicId+"]["+systemId+"]");
     if(systemId.compareTo("http://frontier.fnal.gov/descriptor.dtd")==0)
      {
       StringReader sr=new StringReader(dtd);
       return new InputSource(sr);
      }      
     return null;
    }
  }


public class Counter
    extends DefaultHandler {

    
    
    
  protected static MyResolver myres;
    //
    // Constants
    //

    // feature ids

    /** Validation feature id (http://xml.org/sax/features/validation). */
    protected static final String VALIDATION_FEATURE_ID = "http://xml.org/sax/features/validation";

    /** Default parser name. */
    protected static final String DEFAULT_PARSER_NAME = "org.apache.xerces.parsers.SAXParser";
    
    static int repetition=10;

    //
    // Data 
    //

    /** Number of elements. */
    protected long fElements;

    /** Number of attributes. */
    protected long fAttributes;

    /** Number of characters. */
    protected long fCharacters;

    /** Number of ignorable whitespace characters. */
    protected long fIgnorableWhitespace;

    /** Number of characters of tags. */
    protected long fTagCharacters;

    /** Number of other content characters for the "tagginess" calculation. */
    protected long fOtherCharacters;

    //
    // Constructors
    //

    /** Default constructor. */
    public Counter() {
    } // <init>()

    //
    // Public methods
    //

    /** Prints the results. */
    public void printResults(PrintWriter out, String uri, long time,
                             long memory, boolean tagginess,
                             int repetition) {

        // filename.xml: 631 ms (4 elems, 0 attrs, 78 spaces, 0 chars)
        out.print(uri);
        out.print(": ");
        if (repetition == 1) {
            out.print(time);
        }
        else {
            out.print(time);
            out.print('/');
            out.print(repetition);
            out.print('=');
            out.print(time/repetition);
        }
        out.print(" ms");
        if (memory != Long.MIN_VALUE) {
            out.print(", ");
            out.print(memory);
            out.print(" bytes");
        }
        out.print(" (");
        out.print(fElements);
        out.print(" elems, ");
        out.print(fAttributes);
        out.print(" attrs, ");
        out.print(fIgnorableWhitespace);
        out.print(" spaces, ");
        out.print(fCharacters);
        out.print(" chars)");
        if (tagginess) {
            out.print(' ');
            long totalCharacters = fTagCharacters + fOtherCharacters
                                 + fCharacters + fIgnorableWhitespace;
            long tagValue = fTagCharacters * 100 / totalCharacters;
            out.print(tagValue);
            out.print("% tagginess");
        }
        out.println();
        out.flush();

    } // printResults(PrintWriter,String,long)

    //
    // ContentHandler methods
    //

    /** Start document. */
    public void startDocument() throws SAXException {

        fElements            = 0;
        fAttributes          = 0;
        fCharacters          = 0;
        fIgnorableWhitespace = 0;
        fTagCharacters       = 0;

    } // startDocument()

    /** Start element. */
    public void startElement(String uri, String local, String raw,
                             Attributes attrs) throws SAXException {

        fElements++;
        fTagCharacters++; // open angle bracket
        fTagCharacters += raw.length();
        if (attrs != null) {
            int attrCount = attrs.getLength();
            fAttributes += attrCount;
            for (int i = 0; i < attrCount; i++) {
                fTagCharacters++; // space
                fTagCharacters += attrs.getQName(i).length();
                fTagCharacters++; // '='
                fTagCharacters++; // open quote
                fOtherCharacters += attrs.getValue(i).length();
                fTagCharacters++; // close quote
            }
        }
        fTagCharacters++; // close angle bracket

    } // startElement(String,String,StringAttributes)

    /** Characters. */
    public void characters(char ch[], int start, int length)
        throws SAXException {

        fCharacters += length;

    } // characters(char[],int,int);

    /** Ignorable whitespace. */
    public void ignorableWhitespace(char ch[], int start, int length)
        throws SAXException {

        fIgnorableWhitespace += length;

    } // ignorableWhitespace(char[],int,int);

    /** Processing instruction. */
    public void processingInstruction(String target, String data)
        throws SAXException {
        fTagCharacters += 2; // "<?"
        fTagCharacters += target.length();
        if (data != null && data.length() > 0) {
            fTagCharacters++; // space
            fOtherCharacters += data.length();
        }
        fTagCharacters += 2; // "?>"
    } // processingInstruction(String,String)

    //
    // ErrorHandler methods
    //

    /** Warning. */
    public void warning(SAXParseException ex) throws SAXException {
        printError("Warning", ex);
    } // warning(SAXParseException)

    /** Error. */
    public void error(SAXParseException ex) throws SAXException {
        printError("Error", ex);
        throw new SAXException(ex);
    } // error(SAXParseException)

    /** Fatal error. */
    public void fatalError(SAXParseException ex) throws SAXException {
        printError("Fatal Error", ex);
        throw ex;
    } // fatalError(SAXParseException)

    //
    // Protected methods
    //

    /** Prints the error message. */
    protected void printError(String type, SAXParseException ex) {

        System.err.print("[");
        System.err.print(type);
        System.err.print("] ");
        if (ex== null) {
            System.out.println("!!!");
        }
        String systemId = ex.getSystemId();
        if (systemId != null) {
            int index = systemId.lastIndexOf('/');
            if (index != -1)
                systemId = systemId.substring(index + 1);
            System.err.print(systemId);
        }
        System.err.print(':');
        System.err.print(ex.getLineNumber());
        System.err.print(':');
        System.err.print(ex.getColumnNumber());
        System.err.print(": ");
        System.err.print(ex.getMessage());
        System.err.println();
        System.err.flush();

    } // printError(String,SAXParseException)

    //
    // MAIN
    //

    /** Main program entry point. */
    public static void main(String argv[]) {

        // is there anything to do?
        if (argv.length == 0) {
            printUsage();
            System.exit(1);
        }
        
        myres=new MyResolver();

        // variables
        Counter counter = new Counter();
        PrintWriter out = new PrintWriter(System.out);
        XMLReader parser = null;
        boolean validation = true;

                // create parser
                try {
                    parser = XMLReaderFactory.createXMLReader(DEFAULT_PARSER_NAME);
                    parser.setEntityResolver(myres);
                    System.out.println("Parser name="+DEFAULT_PARSER_NAME+".");
                }
                catch (Exception e) {
                    System.err.println("error: Unable to instantiate parser ("+DEFAULT_PARSER_NAME+")");
                }

            try {
                parser.setFeature(VALIDATION_FEATURE_ID, validation);
            }
            catch (SAXException e) {
                System.err.println("warning: Parser does not support feature ("+VALIDATION_FEATURE_ID+")");
            }

            // parse file
            parser.setContentHandler(counter);
            parser.setErrorHandler(counter);
            try {
                long timeBefore = System.currentTimeMillis();
                long memoryBefore = Runtime.getRuntime().freeMemory();
                for (int j = 0; j < repetition; j++) {
                    parser.parse(argv[0]);
                }
                long memoryAfter = Runtime.getRuntime().freeMemory();
                long timeAfter = System.currentTimeMillis();

                long time = timeAfter - timeBefore;
                long memory = memoryBefore - memoryAfter;
                counter.printResults(out, argv[0], time, memory, true, repetition);
            }
            catch (SAXParseException e) {
                // ignore
            }
            catch (Exception e) {
                System.err.println("error: Parse error occurred - "+e.getMessage());
                Exception se = e;
                if (e instanceof SAXException) {
                    se = ((SAXException)e).getException();
                }
                if (se != null)
                  se.printStackTrace(System.err);
                else
                  e.printStackTrace(System.err);

            }

    } // main(String[])

    //
    // Private static methods
    //

    /** Prints the usage. */
    private static void printUsage() {

        System.err.println("usage: java sax.Counter xsd.xml");
        System.err.println();


    } // printUsage()

} // class Counter
