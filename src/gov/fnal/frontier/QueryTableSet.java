package gov.fnal.frontier;

import java.util.HashSet;
import java.util.Iterator;

// this class extends HashSet just to make a prettier toString() for printing
public class QueryTableSet<T> extends HashSet<T>
 {
  public String toString()
   {
    Iterator it=iterator();
    String name=(String)it.next(); // The first table name
    String str=name.toUpperCase();
    while (it.hasNext()) 
     {
      str+=" and ";
      name=(String)it.next();
      str+=name.toUpperCase();
     }
    return str.replaceAll("\"","");
   }
 }
