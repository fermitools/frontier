package gov.fnal.frontier;

import java.io.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import gov.fnal.frontier.codec.Encoder;


public class XsdDataObject extends DefaultHandler implements FrontierDataObject
 {
  private static final String VALIDATION_FEATURE_ID="http://xml.org/sax/features/validation";
  private static final String DEFAULT_PARSER_NAME="org.apache.xerces.parsers.SAXParser";
  
  private DbConnectionMgr dbm;


  protected XsdDataObject(DbConnectionMgr dbm) throws Exception
   {
    this.dbm=dbm;
   }

      
  public void fdo_init(byte[] body) throws Exception
   {
    String xml=new String(body);
    XMLReader parser=null;
    boolean validation=false;

    parser=XMLReaderFactory.createXMLReader(DEFAULT_PARSER_NAME);
    //parser.setEntityResolver(myres);
    try 
     {
      parser.setFeature(VALIDATION_FEATURE_ID, validation);
     }
    catch(SAXException e) 
     {
      System.out.println("warning: Parser does not support feature ("+VALIDATION_FEATURE_ID+")");
     }
    
    parser.setContentHandler(this);
    parser.setErrorHandler(this);
    StringReader sr=new StringReader(xml);
    InputSource is=new InputSource(sr);
    parser.parse(is);
   }
 
     
  public void fdo_get(OutputStream out,Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
   }
   
   
  public void fdo_meta(OutputStream out,Encoder enc) throws Exception
   {
   }
  
  
  /*
   * SAX related staf below
   */
   
  public void startDocument() throws SAXException 
   {
    System.out.println("Start document");
   }
   
  
  public void startElement(String uri,String local,String raw, Attributes attrs) throws SAXException 
   {
    System.out.println("startElem u="+uri+",l="+local+",r="+raw+",a="+attrs);
    if(attrs!=null) 
     {
      for(int i=0;i<attrs.getLength();i++) 
       {
        String n=attrs.getQName(i);
        String v=attrs.getValue(i);
        System.out.println("attr "+n+":"+v+".");
       }
     }
   }
   
      
  public void characters(char ch[],int start,int length) throws SAXException 
   {
    String s=new String(ch,start,length);
    System.out.println("char ["+s+"]");
   }
   
   
  /** Warning. */
  public void warning(SAXParseException ex) throws SAXException 
   {
    Frontier.Log("Warning", ex);
   }

  /** Error. */
  public void error(SAXParseException ex) throws SAXException 
   {
    Frontier.Log("Error", ex);
    throw new SAXException(ex);
   }

  /** Fatal error. */
  public void fatalError(SAXParseException ex) throws SAXException 
   {
    Frontier.Log("Fatal Error", ex);
    throw ex;
   }   
 }
