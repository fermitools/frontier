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

  // Returns source od DTD fr validating
  class LocalResolver implements EntityResolver 
   {
    // XSD v1 DTD
    private static final String dtd=
     "<!ELEMENT descriptor (attribute+,select,from,where*,final)> "
    +"<!ATTLIST descriptor "
    +" type CDATA #REQUIRED "
    +" version CDATA #REQUIRED "
    +" xsdversion (1) #REQUIRED> "
    +"<!ELEMENT attribute EMPTY> "
    +"<!ATTLIST attribute "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|bytes|date|blob) #REQUIRED "
    +" field CDATA #REQUIRED> "
    +"<!ELEMENT select (#PCDATA)> "
    +"<!ELEMENT from (#PCDATA)> "
    +"<!ELEMENT where (clause,param*)> "
    +"<!ELEMENT clause (#PCDATA)> "
    +"<!ELEMENT param EMPTY> "
    +"<!ATTLIST param "
    +" position CDATA #REQUIRED "
    +" type (int|long|double|float|string|date) #REQUIRED "
    +" key CDATA #REQUIRED> "
    +"<!ELEMENT final (#PCDATA)>";
    
    public InputSource resolveEntity(String publicId,String systemId)
     {
      //System.out.println("["+publicId+"]["+systemId+"]");
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
    StringBuffer clause=new StringBuffer("");
    String str_clause;
    ArrayList aParams=new ArrayList();
    HashMap aMap=new HashMap();
   }// end of WhereClause
   

  protected XsdDataObject(DbConnectionMgr dbm,String requested_name,String requested_version) throws Exception
   {
    this.dbm=dbm;
    obj_name=requested_name;
    obj_version=requested_version;
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
   
   
   
  public MethodDesc fdo_getMethodDesc(String method) throws Exception
   {
    MethodDesc md=new MethodDesc("DEFAULT","get",false,((long)1000*60*60*24*7),"free","public");
    return md;
   }
   
     
   
  public int fdo_get(Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    //System.out.println("XsdDataObject.fdo_get()");
    int rec_num=0;
    
    // XSD v1 does not define "method", so chech it was not defined
    if(method!=null && (!method.equals("DEFAULT"))) throw new Exception("XSD v1 does not support methods");
    
    WhereClause wc=find_wehere_clause(fds);
    if(wc==null) throw new Exception("Can not find suitable where clause for object "+obj_name+" in domain GET");
    
    StringBuffer sql=new StringBuffer("");
    sql.append(main_sql);
    if(wc.str_clause.length()>0)
     {
      sql.append(" where ");
      sql.append(wc.str_clause);
     }      
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
        FieldDesc param=(FieldDesc)wc.aParams.get(i);
        String val=fds.getString(param.n);
        //System.out.println("Param "+i+" ["+param.n+":"+val+"]");
        stmt.setString(i+1,val);
       }
      rs=stmt.executeQuery();
      while(rs.next())
       {
        for(int i=0;i<aFields.size();i++)
         {
          FieldDesc field=(FieldDesc)aFields.get(i);
          switch(field.t)
           {
            case FieldDesc.F_INT:    enc.writeInt(rs.getInt(i+1)); break;
            case FieldDesc.F_LONG:   enc.writeLong(rs.getLong(i+1)); break;
            case FieldDesc.F_DOUBLE: enc.writeDouble(rs.getDouble(i+1)); break;
            case FieldDesc.F_FLOAT:  enc.writeFloat(rs.getFloat(i+1)); break;
            case FieldDesc.F_STRING: enc.writeString(rs.getString(i+1)); break;
            case FieldDesc.F_BYTES:  enc.writeBytes(rs.getBytes(i+1)); break;  //bytes
            case FieldDesc.F_DATE:   enc.writeDate(rs.getDate(i+1)); break;
            case FieldDesc.F_BLOB: 
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
   
   
  public int fdo_meta(Encoder enc,String method) throws Exception
   {
    // Method is ignored for XSD v1
    //System.out.println("XsdDataObject.fdo_meta()");
    for(int i=0;i<aFields.size();i++)
     {
      FieldDesc field=(FieldDesc)aFields.get(i);
      enc.writeString(FieldDesc.type_name[field.t]);
      enc.writeString(field.n);
     }
    enc.writeEOR();
    return 1; // Just one record
   }
     
  
  public int fdo_write(Encoder enc,String method,FrontierDataStream fds) throws Exception
   {
    throw new Exception("XSD v1 does not support writes");
   }   
  
  
   
  private WhereClause find_wehere_clause(FrontierDataStream fds)
   {
    Object[] keys=fds.getParamNames();
    WhereClause ret=null;
    
    for(int i=0;i<aWhere.size();i++)
     {
      WhereClause wc=(WhereClause)aWhere.get(i);
      if(wc.aParams.size()!=keys.length) continue;
      
      boolean failed=false;
      for(int n=0;n<keys.length;n++)
       {
        if(!wc.aMap.containsKey((String)keys[n])) {failed=true; break;}
       }
      if(failed) continue;
      // This is it!
      ret=wc;
      break;
     }     
    return ret;
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
      //System.out.println("attr "+attrs.getQName(i)+":"+attrs.getValue(i)+".");
     }
   }
   
   
  public void startDocument() throws SAXException 
   {
    //System.out.println("Start document");
   }
   
  
  public void startElement(String uri,String local,String raw, Attributes attrs) throws SAXException 
   {
    //System.out.println("startElem u="+uri+",l="+local+",r="+raw+".");

    // print_attrs(attrs);
        
    if(local.equals("descriptor"))
     {
      String tmp=attrs.getValue("type");
      if(!obj_name.equals(tmp)) throw new SAXException("Object name mismatch: expected ["+obj_name+"], got "+tmp+"]");
      tmp=attrs.getValue("version");
      if(!obj_version.equals(tmp)) throw new SAXException("Object version mismatch: expected ["+obj_version+"], got "+tmp+"]");
      xsd_version=attrs.getValue("xsdversion");
      //System.out.println("xsd_version="+xsd_version+".");
      if(Integer.parseInt(xsd_version)!=1) throw new SAXException("XSD version mismatch - expected 1, got "+xsd_version);
      return;
     }
     
    if(local.equals("attribute"))
     {
      FieldDesc field=new FieldDesc(attrs.getValue("field"),attrs.getValue("type"));
      int pos=Integer.parseInt(attrs.getValue("position"));
      if(aFields.size()>=pos) throw new SAXException("Attributes are out of order!");
      
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
      return;
     }
     
    if(current_element.equals("param"))
     {
      // The last one is the current
      WhereClause wc=(WhereClause)aWhere.get(aWhere.size()-1);
      FieldDesc param=new FieldDesc(attrs.getValue("key"),attrs.getValue("type"));
      int pos=Integer.parseInt(attrs.getValue("position"));
      if(wc.aParams.size()>=pos) throw new SAXException("Params are out of order!");
      wc.aParams.add(param);
      wc.aMap.put(param.n,new Integer(pos));
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
      wc.str_clause=wc.clause.toString().trim();
      System.out.println("New clause ["+wc.clause.toString()+"]");      
      // XSD v1 does not define "method", so here is no map_by_method
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
