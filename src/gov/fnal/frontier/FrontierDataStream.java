package gov.fnal.frontier;


public class FrontierDataStream
 {
  public static final int BY_NAME=0;
  
  private int mode;
 
  public FrontierDataStream(int mode)
   {
    this.mode=mode;
   }
   
   
  public void append(String name,String val)
   {
    System.out.println("Ignored: "+name+":"+val);
   }
 }
