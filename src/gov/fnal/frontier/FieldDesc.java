/**
 * Manage a field descriptor
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import java.util.*;
import org.xml.sax.*;

public class FieldDesc
 {
  public static final int F_INT=0;
  public static final int F_LONG=1;
  public static final int F_DOUBLE=2;
  public static final int F_FLOAT=3;
  public static final int F_STRING=4;
  public static final int F_BYTES=5;
  public static final int F_DATE=6;
  public static final int F_BLOB=7;
  
  public static final String[] type_name=
   {
    "int",
    "long",
    "double",
    "float",
    "string",
    "bytes",
    "date",
    "blob"
   };
   
  public static HashMap<String,Integer> mapType;
  
  static
   {
    mapType=new HashMap<String,Integer>();
    for(int i=0;i<type_name.length;i++)
     {
      mapType.put(type_name[i],new Integer(i));
     }
   }

  protected String n; // Field name    
  protected int t;    // Field type
  
  protected FieldDesc(String field,String type) throws SAXException
   {
    Integer it=(Integer)mapType.get(type);
    if(it==null) throw new SAXException("Unknown type "+type+" of field "+field);
    t=it.intValue();
    n=field;
   }
 }

 
