package gov.fnal.frontier;

import java.util.*;

public class FrontierDataStream
 {
  public static final int BY_NAME=0;
  
  private int mode;
  private HashMap mapParam;
  
 
  public FrontierDataStream(int mode)
   {
    this.mode=mode;
    mapParam=new HashMap();
   }
   
   
  public void append(String name,String val)
   {
    mapParam.put(name,val);
    System.out.println("Added: "+name+":"+val);    
   }
   
   
  public String getString(String key) throws Exception
   {
    if(mode!=BY_NAME) throw new Exception("Wromg mode");
    
    return (String)mapParam.get(key);
   }
 }
