package gov.fnal.frontier.fdo;

import java.util.*;

public class FrontierDataStream
 {
  public static final int BY_NAME=0;
  
  private int mode;
  private HashMap mapParam;
  private int count;
  
 
  public FrontierDataStream(int mode)
   {
    this.mode=mode;
    mapParam=new HashMap();
    count=0;
   }
   
   
  public void append(String name,String val) throws Exception
   {
    if(mapParam.containsKey(name)) throw new Exception ("Parameter "+name+" is already defined");
    mapParam.put(name,val);
    ++count;
    //System.out.println("Added: "+name+":"+val);
   }
   
   
  public int getParamNum()
   {
    return count;
   }
   
   
  public Object[] getParamNames()
   {
    Object[] ret=mapParam.keySet().toArray();
    return ret;
   }
   
   
  public String getString(String key) throws Exception
   {
    if(mode!=BY_NAME) throw new Exception("Wromg mode");
    
    String ret=(String)mapParam.get(key);
    if(ret==null) throw new Exception("Required parameter "+key+" is not defiled.");
    return ret;
   }
 }
