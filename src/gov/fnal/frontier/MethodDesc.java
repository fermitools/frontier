package gov.fnal.frontier;

import java.util.*;

public class MethodDesc
 {
  protected String name;
  protected String domain;
  protected boolean noCache;
  protected long expire;
  protected String transaction;
  protected String access;
  protected StringBuffer sql_buf;
  protected String sql_str;
  protected String sql_type;
  protected ArrayList aParams;
  
  protected MethodDesc()
   {
    aParams=new ArrayList();
   }
   
  //public String getName() {return name;}
  //public String getDomain() {return domain;}
  public boolean isNoCache() {return noCache;}
  public long getExpire() {return expire;}
  //public String getTransaction() {return transaction;}
  //public String getAccess() {return access;}
  //public String getSql() {return sql.toString();}
  //public String getSqlType() {return sql_type;}
  
/*  public int getParamNum() 
   {
    return aParams.size();
   }
  
  public String getParamName(int ind)
   {
    String[] p=(String[])aParams.get(ind);
    return p[1];
   }

   
  public char getParamType(int ind)
   {
    String[] p=(String[])aParams.get(ind);
    return p[0].charAt(0);
   }*/
 }
