package gov.fnal.frontier;

import java.io.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import gov.fnal.frontier.codec.Encoder;
import java.util.*;
import java.sql.*;



public class XsdDataObject extends DefaultHandler implements FrontierDataObject
 {
  private static final String VALIDATION_FEATURE_ID="http://xml.org/sax/features/validation";
  private static final String DEFAULT_PARSER_NAME="org.apache.xerces.parsers.SAXParser";
  
  // The first version of XSD did not included standard header and reference to 
  // DTD, so I add it here manually
  private static final String xml_header=
    "<?xml version=\"1.0\" encoding=\"US-ASCII\"?> "
   +"<!DOCTYPE descriptor SYSTEM \"http://frontier.fnal.gov/descriptor.dtd\"> ";
  
  private DbConnectionMgr dbm;
  
  // Current state for SAX parser
  private String current_element;
  
  // Public stuff
  public String obj_name;
  public String obj_version;
  public String xsd_version;
  public ArrayList aFields;
  public StringBuffer main_sql;
  public StringBuffer tail_sql;
  public ArrayList aWhere;
  public HashMap mapWhere;

  // Returns source od DTD fr validating
  class LocalResolver implements EntityResolver 
   {
    // XSD v1 DTD
    private static final String dtd=
     "<!ELEMENT descriptor (attribute+,select,from,where*,final)> "
    +"<!ATTLIST descriptor "
    +" type CDATA #REQUIRED "
    +" version CDATA #REQUIRED "
    +" xsdversion CDATA #REQUIRED> "
    +"<!ELEMENT attribute EMPTY> "
    +"<!ATTLIST attribute "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|bytes|date) #REQUIRED "
    +" field CDATA #REQUIRED> "
    +"<!ELEMENT select (#PCDATA)> "
    +"<!ELEMENT from (#PCDATA)> "
    +"<!ELEMENT where (clause,param*)> "
    +"<!ATTLIST where "
    +" method CDATA #FIXED \"DEFAULT\"> "
    +"<!ELEMENT clause (#PCDATA)> "
    +"<!ELEMENT param EMPTY> "
    +"<!ATTLIST param "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|date) #REQUIRED "
    +" key CDATA #REQUIRED> "
    +"<!ELEMENT final (#PCDATA)>";
    
    public InputSource resolveEntity(String publicId,String systemId)
     {
      System.out.println("["+publicId+"]["+systemId+"]");
      if(systemId.equals("http://frontier.fnal.gov/descriptor.dtd"))
       {
        StringReader sr=new StringReader(dtd);
        return new InputSource(sr);
       }      
      return null;
     }
   }// End of LocalResolver
  
   
  // WhereClause for selection in XSD v1
  class WhereClause
   {
    StringBuffer clause=new StringBuffer(" where ");;
    ArrayList aParams=new ArrayList();
    HashMap aMap=new HashMap();
   }// end of WhereClause
   

  protected XsdDataObject(DbConnectionMgr dbm) throws Exception
   {
    this.dbm=dbm;
   }

   
  private String prepareXml(byte[] body) throws Exception
   {
    // StringBuffer can not append byte[] - isn't it silly?
    String str=new String(body,"US-ASCII");
    StringBuffer sbuf=new StringBuffer(xml_header);
    sbuf.append(str);
    return sbuf.toString();   
   }
   
      
  public void fdo_init(byte[] body) throws Exception
   {
    String xml=prepareXml(body);
    XMLReader parser=null;
    boolean validation=true;
    aFields=new ArrayList();
    current_element=null;
    main_sql=new StringBuffer("select ");
    tail_sql=new StringBuffer("");
    aWhere=new ArrayList();
    mapWhere=new HashMap();

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
 
  
  public long fdo_get_expiration_time()
   {
    return (1000 * 60 * 60 * 24 * 7); // 7 days, in milliseconds
   }
   
   
  public boolean fdo_is_no_cache()
   {
    return false;
   }
   
   
  public int fdo_get(Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    System.out.println("XsdDataObject.fdo_get()");
    int rec_num=0;
    
    WhereClause wc=(WhereClause)mapWhere.get(method);
    if(wc==null) throw new Exception("Methos "+method+" is not defined for object "+obj_name+" in domain GET");
    
    StringBuffer sql=new StringBuffer("");
    sql.append(main_sql);
    sql.append(" ");
    sql.append(wc.clause);
    sql.append(" ");
    sql.append(tail_sql);
    System.out.println("sql ["+sql.toString()+"]");
    
    Connection con=null;
    PreparedStatement stmt=null;
    ResultSet rs=null;
    
    try
     {
      con=dbm.acquire();
      stmt=con.prepareStatement(sql.toString());
      for(int i=0;i<wc.aParams.size();i++)
       {
        String[] param=(String[])wc.aParams.get(i);
        String val=fds.getString(param[1]);
        System.out.println("Param "+i+" ["+param[1]+":"+val+"]");
        stmt.setString(i+1,val);
       }
      rs=stmt.executeQuery();
      while(rs.next())
       {
        for(int i=0;i<aFields.size();i++)
         {
          String[] field=(String[])aFields.get(i);
          switch(field[0].charAt(0))
           {
            case 'i': enc.writeInt(rs.getInt(i+1)); break;
            case 'l': enc.writeLong(rs.getLong(i+1)); break;
            case 'd': enc.writeDouble(rs.getDouble(i+1)); break;
            case 'f': enc.writeFloat(rs.getFloat(i+1)); break;
            case 's': enc.writeString(rs.getString(i+1)); break;
            case 'D': enc.writeDate(rs.getDate(i+1)); break;
            case 'b': 
             Blob blob=rs.getBlob(i+1);
             int len=(int)blob.length();
             byte[] b=blob.getBytes((long)1,len);
             enc.writeBytes(b);
             break;
            default: throw new Exception("Type adjustment is needed");
           }
         }
        rec_num++;
        enc.writeEOR();
       }
     }
    finally
     {
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
      if(con!=null) try{dbm.release(con);}catch(Exception e){}
     }
    return rec_num;
   }
   
   
  public int fdo_meta(Encoder enc) throws Exception
   {
    System.out.println("XsdDataObject.fdo_meta()");
    for(int i=0;i<aFields.size();i++)
     {
      String[] field=(String[])aFields.get(i);
      enc.writeString(field[1]);
      enc.writeString(field[2]);
     }
    enc.writeEOR();
    return 1; // Just one record
   }
  
  
  /********************************************
   ********************************************
   * SAX related stuff is below   
   *
   ********************************************/   
   
   
  private void print_attr(Attributes attrs) throws SAXException
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
    System.out.println("startElem u="+uri+",l="+local+",r="+raw+".");

    // print_attrs(attrs);
        
    if(local.equals("descriptor"))
     {
      obj_name=attrs.getValue("type");
      obj_version=attrs.getValue("version");
      xsd_version=attrs.getValue("xsdversion");
      System.out.println("xsd_version="+xsd_version+".");
      if(Integer.parseInt(xsd_version)!=1) throw new SAXException("XSD version mismatch - expected 1, got "+xsd_version);
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
     
    current_element=local;
    
    if(current_element.equals("from"))
     {
      main_sql.append(" from ");
      return;
     }
     
    if(current_element.equals("where"))
     {
      WhereClause wc=new WhereClause();
      aWhere.add(wc);
      String method=attrs.getValue("method");
      System.out.println("method="+method);
      if(mapWhere.containsKey(method)) throw new SAXException("Method "+method+" is already defined");
      mapWhere.put(method,wc);
      return;
     }
     
    if(current_element.equals("param"))
     {
      // The last one is the current
      WhereClause wc=(WhereClause)aWhere.get(aWhere.size()-1);
      String[] param=new String[2];
      int pos=Integer.parseInt(attrs.getValue("position"));
      if(wc.aParams.size()>=pos) throw new SAXException("Params are out of order!");
      // Replace full type names with one-char acronyms, so we could use "switch" later
      String t=attrs.getValue("type");
      if(t.equals("int") || t.equals("long") || t.equals("double") || t.equals("float") || t.equals("string"))
       param[0]=t.substring(0,1);
      else if(t.equals("date")) param[0]="D";
      else throw new SAXException("Type set for params needs adjustment for "+t);
      
      param[1]=attrs.getValue("key");
      wc.aParams.add(param);
      wc.aMap.put(param[1],new Integer(pos));
      return;      
     }
   }
   
   
   
  public void endElement(String uri,String local,String qName) throws SAXException
   {
    current_element=null;
    
    if(local.equals("where")) 
     {
      WhereClause wc=(WhereClause)aWhere.get(aWhere.size()-1);
      while(true)
       {
        int pos=wc.clause.indexOf("@param");
        if(pos<0) break;
        wc.clause.replace(pos,pos+6,"?");
       }
      System.out.println("New clause ["+wc.clause.toString()+"]");
     }
   }
   
   
      
  public void characters(char ch[],int start,int length) throws SAXException 
   {
    if(current_element==null) return;
    if(current_element.equals("select"))
     {
      main_sql.append(ch,start,length);
      return;
     }
    if(current_element.equals("from"))
     {
      main_sql.append(ch,start,length);
      return;
     }
    if(current_element.equals("clause"))
     {
      // The last one is the current
      WhereClause wc=(WhereClause)aWhere.get(aWhere.size()-1);
      wc.clause.append(ch,start,length);
      return;
     }
    if(current_element.equals("final"))
     {
      tail_sql.append(ch,start,length);
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
