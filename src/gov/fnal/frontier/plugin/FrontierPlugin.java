package gov.fnal.frontier.plugin;

import gov.fnal.frontier.fdo.*;

public interface FrontierPlugin
 {
  public String[] fp_getMethods();
  public MethodDesc fp_getMethodDesc(String name) throws Exception;
  public int fp_get(java.sql.Connection con,Encoder enc,String method,FrontierDataStream fds) throws Exception;
  public int fp_meta(Encoder enc,String method) throws Exception;
  public int fp_write(java.sql.Connection con,Encoder enc,String method,FrontierDataStream fds) throws Exception;  
 }
