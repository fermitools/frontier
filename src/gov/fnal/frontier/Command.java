package gov.fnal.frontier;

import java.util.*;
import javax.servlet.http.*;


public class Command 
 {
  public static final int CMD_ADMIN=0;
  public static final int CMD_GET=1;
  public static final int CMD_UPDATE=2;
  public static final int CMD_META=3;
  public static final String[] domain_name={"admin","get","update","meta"};
  
  public int cmd_domain;
  public String obj_name;
  public String obj_version;
  public String command;
  public String method;
  public String encoder;
  public FrontierDataStream fds;
  
  private Command()
   {
   }
 
   
  public static ArrayList parse(HttpServletRequest req) throws Exception
   {
    ArrayList ret=new ArrayList();
    
    String req_method=req.getMethod();
    if(req_method.equals("GET"))
     {
      set_get_command(ret,req.getQueryString());
     }
    else if(req_method.equals("POST"))
     {
      throw new Exception("POST is not implemented yet!");
     }
    else
     {
      throw new Exception("Unsupported method ["+req_method+"]");
     }
    return ret;
   }  
   
   
   
  private static void set_get_command(ArrayList cmd_list,String query_str) throws Exception
   {
    System.out.println("Query string ["+query_str+"]");
    String[] param=new String[4];
    Object[] env=new Object[2];
        
    boolean completed=get_next_param(param,env,query_str);
    
    while(!completed)
     {
      Command c=new Command();
      
      System.out.println("p0=["+param[0]+"]");
      
      if(param[0].equals("type"))
       {
        c.cmd_domain=CMD_GET;
        c.obj_name=param[1];
        c.obj_version=param[2];
        c.command="get";
        c.method=param[3];
        if(c.method==null) c.method="DEFAULT";
       }
      else if(param[0].equals("meta"))
       {
        c.cmd_domain=CMD_META;
        c.obj_name=param[1];
        c.obj_version=param[2];
        c.command=""; // Does not matter
        c.method="";  // Does not matter
       }
      else
       {
        throw new Exception("Unsupported request type ["+param[0]+"]");
       }
       
      if(get_next_param(param,env,query_str)) throw new Exception("Pass incomplete - 4th down, 10 yds to go!");
      
      if(param[0].equals("encoding")) c.encoder=param[1];
      else throw new Exception("Unexpected parameter ["+param[0]+"="+param[1]+"]");
              
      // Get parameters list
      c.fds=new FrontierDataStream(FrontierDataStream.BY_NAME);
      while(true)
       {
        completed=get_next_param(param,env,query_str);
        if(completed) break;
        if(param[0].equals("type") || param[0].equals("meta")) break;
        c.fds.append(param[0],param[1]);
       }
      cmd_list.add(c);      
     }
   }
   

   
  private static boolean get_next_param(String[] param,Object[] env,String str) throws Exception
   {
    param[0]=null;
    param[1]=null;
    param[2]=null;
    param[3]=null;

    // getParameterNames()'s result order is undefined, so do it manually
    StringTokenizer st=(StringTokenizer)env[0];
    if(st==null) 
     {
      st=new StringTokenizer(str,"&");
      env[0]=st;
     }
    if(!st.hasMoreTokens()) return true;
    String token=st.nextToken();
    
    st=new StringTokenizer(token,"=");
    String par=st.nextToken(); par=java.net.URLDecoder.decode(par,"US-ASCII");
    String val=st.nextToken(); val=java.net.URLDecoder.decode(val,"US-ASCII");
    
    param[0]=par;    
    if(par.equals("type") || par.equals("meta"))
     {
      st=new StringTokenizer(val,":");
      param[1]=st.nextToken();
      if(st.hasMoreTokens()) param[2]=st.nextToken();
      if(st.hasMoreTokens()) param[3]=st.nextToken();
     }
    else
     {
      param[1]=val;
     }
    return false;
   }
   
     
   
  public String toString()
   {
    String ret="d="+domain_name[cmd_domain]+",on="+obj_name+",ov="+obj_version+",c="+command;
    ret+=",m="+method+",e="+encoder+",pl=["+fds+"]";
    return ret;
   }
 }
