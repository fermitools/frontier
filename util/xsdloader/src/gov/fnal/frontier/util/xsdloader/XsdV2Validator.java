package gov.fnal.frontier.util.xsdloader;

import java.io.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import java.util.*;



public class XsdV2Validator extends DefaultHandler implements XsdValidator
 {
  private static final String VALIDATION_FEATURE_ID="http://xml.org/sax/features/validation";
  private static final String DEFAULT_PARSER_NAME="org.apache.xerces.parsers.SAXParser";
    
  // Public stuff
  public String obj_name;
  public String obj_version;
  public String xsd_version;

  // Returns source od DTD for validating
  class LocalResolver implements EntityResolver 
   {
    // XSD v2 DTD
    private static final String dtd=
     "<!ELEMENT descriptor (attribute+,method+)> "
    +"<!ATTLIST descriptor "
    +" type CDATA #REQUIRED "
    +" version CDATA #REQUIRED "
    +" xsdversion (2) #REQUIRED> "
    +"<!ELEMENT attribute EMPTY> "
    +"<!ATTLIST attribute "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|bytes|date|blob) #REQUIRED "
    +" field CDATA #REQUIRED> "
    +"<!ELEMENT method (sql,param*)> "
    +"<!ATTLIST method "
    +" name CDATA \"DEFAULT\" "
    +" domain (get|insert|update) #REQUIRED "    
    +" cache-control (cache|no-cache) \"cache\" "
    +" expire CDATA \"604800000\" "
    +" transaction (free|required) #REQUIRED "
    +" access (public|certificate|admin) #REQUIRED> "
    +"<!ELEMENT sql (#PCDATA)> "
    +"<!ATTLIST sql "
    +" type (query|call|update) #REQUIRED> "
    +"<!ELEMENT param EMPTY> "
    +"<!ATTLIST param "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|bytes|date|blob) #REQUIRED "
    +" key CDATA #REQUIRED> ";
    
    public InputSource resolveEntity(String publicId,String systemId)
     {
      System.out.println("["+publicId+"]["+systemId+"]");
      if(systemId.equals("http://frontier.fnal.gov/xsd2.dtd"))
       {
        //System.out.println("DTD:[\n"+dtd+"\n]");
        StringReader sr=new StringReader(dtd);
        return new InputSource(sr);
       }      
      return null;
     }
   }// End of LocalResolver
   
   
     
  protected XsdV2Validator(String requested_name,String requested_version)
   {
    obj_name=requested_name;
    obj_version=requested_version;
   }

   
  private String prepareXml(byte[] body) throws Exception
   {
    // StringBuffer can not append byte[] - isn't it silly?
    String str=new String(body,"US-ASCII");
    //System.out.println("XML:[\n"+str+"\n]");
    return str;
   }
   
   
  public void init(byte[] body) throws Exception
   {
    String xml=prepareXml(body);
    XMLReader parser=null;
    boolean validation=true;
    
    parser=XMLReaderFactory.createXMLReader(DEFAULT_PARSER_NAME);
    LocalResolver lres=new LocalResolver();
    parser.setEntityResolver(lres);
    try 
     {
      parser.setFeature(VALIDATION_FEATURE_ID, validation);
     }
    catch(SAXException e) 
     {
      throw new Exception("Error: Parser does not support feature ("+VALIDATION_FEATURE_ID+"): "+e);
     }
    
    parser.setContentHandler(this);
    parser.setErrorHandler(this);
    StringReader sr=new StringReader(xml);
    InputSource is=new InputSource(sr);
    parser.parse(is);    
   }
   
  /********************************************
   ********************************************
   * SAX related stuff is below   
   *
   ********************************************/   
   
   
  private void print_attrs(Attributes attrs) throws SAXException
   {
    for(int i=0;attrs!=null && i<attrs.getLength();i++) 
     {
      System.out.println("attr "+attrs.getQName(i)+":"+attrs.getValue(i)+".");
     }
   }
   
   
  public void startDocument() throws SAXException 
   {
    System.out.println("Start document");
   }
   
  
  public void startElement(String uri,String local,String raw, Attributes attrs) throws SAXException 
   {
    //System.out.println("startElem u="+uri+",l="+local+",r="+raw+".");

    //print_attrs(attrs);
        
    if(local.equals("descriptor"))
     {
      String tmp=attrs.getValue("type");
      if(!obj_name.equals(tmp)) throw new SAXException("Object name mismatch: expected ["+obj_name+"], got "+tmp+"]");
      tmp=attrs.getValue("version");
      if(!obj_version.equals(tmp)) throw new SAXException("Object version mismatch: expected ["+obj_version+"], got "+tmp+"]");
      xsd_version=attrs.getValue("xsdversion");
      System.out.println("xsd_version="+xsd_version+".");
      if(Integer.parseInt(xsd_version)!=2) throw new SAXException("XSD version mismatch - expected 2, got "+xsd_version);
      return;
     }     
   }
   
   
  /** Warning. */
  public void warning(SAXParseException ex) throws SAXException 
   {
    System.out.println("Warning: "+ex);
    ex.printStackTrace();
   }

  /** Error. */
  public void error(SAXParseException ex) throws SAXException 
   {
    System.out.println("Error: "+ex);
    throw ex;
   }

  /** Fatal error. */
  public void fatalError(SAXParseException ex) throws SAXException 
   {
    System.out.println("Fatal: "+ex);
    throw ex;
   }   
 }
