package gov.fnal.frontier;

import java.io.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import java.util.*;



public class Xsd2DataObjectHelper extends DefaultHandler
 {
  private static final String VALIDATION_FEATURE_ID="http://xml.org/sax/features/validation";
  private static final String DEFAULT_PARSER_NAME="org.apache.xerces.parsers.SAXParser";
    
  // Current state for SAX parser
  private String current_element;
  private MethodDesc current_method;
  
  // Public stuff
  public String obj_name;
  public String obj_version;
  public String xsd_version;
  public ArrayList aFields;
  public HashMap mapMethods;

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
    +" type (int|long|double|float|string|bytes|date) #REQUIRED "
    +" field CDATA #REQUIRED> "
    +"<!ELEMENT method (sql,param*)> "
    +"<!ATTLIST method "
    +" name CDATA \"DEFAULT\" "
    +" domain (get|insert|update) #REQUIRED "    
    +" cache-control (cache|no-cache) \"cache\" "
    +" expire CDATA \"604800000\" "
    +" transaction (free|required) #REQUIRED "
    +" access (public|certificate) #REQUIRED> "
    +"<!ELEMENT sql (#PCDATA)> "
    +"<!ATTLIST sql "
    +" type (query|call|update) #REQUIRED> "
    +"<!ELEMENT param EMPTY> "
    +"<!ATTLIST param "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|date) #REQUIRED "
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
  
   
  protected Xsd2DataObjectHelper(String requested_name,String requested_version) throws Exception
   {
    obj_name=requested_name;
    obj_version=requested_version;
    aFields=new ArrayList();
    mapMethods=new HashMap();
   }

   
  private String prepareXml(byte[] body) throws Exception
   {
    // StringBuffer can not append byte[] - isn't it silly?
    String str=new String(body,"US-ASCII");
    //System.out.println("XML:[\n"+str+"\n]");
    return str;
   }
   
   
  protected void init(byte[] body) throws Exception
   {
    String xml=prepareXml(body);
    XMLReader parser=null;
    boolean validation=true;
    current_element=null;
    current_method=null;
    
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
   
   
  protected MethodDesc getMethodDesc(String method) throws Exception
   {
    MethodDesc md=(MethodDesc)mapMethods.get(method);
    if(md==null) throw new Exception("Method "+method+" was not found for object "+obj_name+":"+obj_version+".");
    return md;
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
     
    if(local.equals("attribute"))
     {
      String[] field=new String[3];
      int pos=Integer.parseInt(attrs.getValue("position"));
      if(aFields.size()>=pos) throw new SAXException("Attributes are out of order!");
      
      // Replace full type names with one-char acronyms, so we could use "switch" later
      String t=attrs.getValue("type");
      if(t.equals("int") || t.equals("long") || t.equals("double") || t.equals("float") || t.equals("string") || t.equals("bytes"))
       field[0]=t.substring(0,1);
      else if(t.equals("date")) field[0]="D";
      else throw new SAXException("Type set needs adjustment for "+t);
      
      field[1]=attrs.getValue("field");
      field[2]=t;
      aFields.add(field);
      return;
     }
     
    if(local.equals("method"))
     {
      current_method=new MethodDesc();
      current_method.name=attrs.getValue("name");
      current_method.domain=attrs.getValue("domain");
      current_method.noCache=(attrs.getValue("cache-control")).equals("no-cache");
      current_method.expire=Long.parseLong(attrs.getValue("expire"));
      current_method.transaction=attrs.getValue("transaction");
      current_method.access=attrs.getValue("access");
      return;
     }    
     
    current_element=local;
    
    if(current_element.equals("sql"))
     {
      current_method.sql_type=attrs.getValue("type");
      current_method.sql_buf=new StringBuffer("");
      return;
     }
     
    if(current_element.equals("param"))
     {
      String[] param=new String[2];
      int pos=Integer.parseInt(attrs.getValue("position"));
      if(current_method.aParams.size()>=pos) throw new SAXException("Params are out of order!");
      // Replace full type names with one-char acronyms, so we could use "switch" later
      String t=attrs.getValue("type");
      if(t.equals("int") || t.equals("long") || t.equals("double") || t.equals("float") || t.equals("string"))
       param[0]=t.substring(0,1);
      else if(t.equals("date")) param[0]="D";
      else throw new SAXException("Type set for params needs adjustment for "+t);
      
      param[1]=attrs.getValue("key");
      current_method.aParams.add(param);
      return;      
     }
   }
   
   
   
  public void endElement(String uri,String local,String qName) throws SAXException
   {
    current_element=null;
    
    if(local.equals("method"))
     {
      mapMethods.put(current_method.name,current_method);
      current_method=null;
      return;
     }
    
    if(local.equals("sql"))
     {
      // Replace '@param' with '?'
      while(true)
       {
        int pos=current_method.sql_buf.indexOf("@param");
        if(pos<0) break;
        current_method.sql_buf.replace(pos,pos+6,"?");
       }
      current_method.sql_str=current_method.sql_buf.toString();
      System.out.println("New SQL ["+current_method.sql_str+"]");      
     }
   }
   
   
      
  public void characters(char ch[],int start,int length) throws SAXException 
   {
    if(current_element==null) return;
    if(current_element.equals("sql"))
     {
      current_method.sql_buf.append(ch,start,length);
      return;
     }
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
