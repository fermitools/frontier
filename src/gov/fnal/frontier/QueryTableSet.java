/**
 * Extension to HashSet to make a more readable toString for queries
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

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
