package gov.fnal.frontier;


public class MethodDesc
 {
  protected String name;
  protected String domain;
  protected boolean noCache;
  protected long expire;
  protected String transaction;
  protected String access;
  
  protected MethodDesc()
   {
   }

  
  protected MethodDesc(String n,String d,boolean c,long e,String t,String a)
   {
    name=n;
    domain=d;
    noCache=c;
    expire=e;
    transaction=t;
    access=a;
   }
         
  public String getName() {return name;}
  public String getDomain() {return domain;}
  public boolean isNoCache() {return noCache;}
  public long getExpire() {return expire;}
  public String getTransaction() {return transaction;}
  public String getAccess() {return access;}
 }
