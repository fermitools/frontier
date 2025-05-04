/**
 * Method descriptor
 * $Id$
 * 
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier.fdo;

import java.util.*;

public class MethodDesc
 {
  protected String name;
  protected String domain;
  protected boolean noCache;
  protected long expire;
  protected String transaction;
  protected String access;
  
  private static HashMap<String,String> hm_domain;
  private static HashMap<String,String> hm_transaction;
  private static HashMap<String,String> hm_access;
  
  static
   {
    hm_domain=new HashMap<String,String>();
    hm_domain.put("get","1");
    hm_domain.put("insert","2");
    hm_domain.put("update","3");
    
    hm_transaction=new HashMap<String,String>();
    hm_transaction.put("free","1");
    hm_transaction.put("required","2");
    
    hm_access=new HashMap<String,String>();
    hm_access.put("public","1");
    hm_access.put("certificate","2");
    hm_access.put("admin","3");
   };
  
   
  private MethodDesc()
   {
   }

  
  public MethodDesc(String n,String d,boolean c,long e,String t,String a) throws Exception
   {
    if(n==null) throw new Exception("Name can not be null");
    name=n;
    if(hm_domain.get(d)==null) throw new Exception("Wrong domain "+d);
    domain=d;
    noCache=c;
    expire=e;
    if(hm_transaction.get(t)==null) throw new Exception("Wrong transaction spec "+t);
    transaction=t;
    if(hm_access.get(a)==null) throw new Exception("Wrong access type "+a);
    access=a;
   }
         
  public String getName() {return name;}
  public String getDomain() {return domain;}
  public boolean isNoCache() {return noCache;}
  public long getExpire() {return expire;}
  public String getTransaction() {return transaction;}
  public String getAccess() {return access;}
  
  public MethodDesc copy()
   {
    MethodDesc ret=new MethodDesc();
    ret.name=name;
    ret.domain=domain;
    ret.noCache=noCache;
    ret.expire=expire;
    ret.transaction=transaction;
    ret.access=access;
    return ret;
   }
 }
