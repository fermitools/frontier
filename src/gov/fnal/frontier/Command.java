/**
 * Handle http command
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import java.util.*;
import jakarta.servlet.http.*;
import gov.fnal.frontier.fdo.*;


public class Command 
 {
  public static final int CMD_ADMIN=0;
  public static final int CMD_GET=1;
  public static final int CMD_WRITE=2;
  public static final int CMD_META=3;
  public static final String[] domain_name={"admin","get","update","meta"};
  
  public int cmd_domain;
  public String query;
  public String obj_name;
  public String obj_version;
  public String method;
  public String encoder;
  public FrontierDataStream fds;
  
  private Command()
   {
   }
 
   
  public static ArrayList<Command> parse(HttpServletRequest req) throws Exception
   {
    ArrayList<Command> ret=new ArrayList<Command>();
    
    String req_method=req.getMethod();
    if(req_method.equals("GET"))
     {
      String com=req.getQueryString();
      if(com==null)
       {
	// URL had slash instead of question mark
	// Note: in order for getPathInfo to work, WEB-INF/web.xml has to have
	//   <url-pattern>/Frontier/*</url-pattern>
	// instead of 
	//   <url-pattern>/Frontier</url-pattern>
        com=req.getPathInfo();
       }
      set_get_command(ret,com);
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
   
   
   
  private static void set_get_command(ArrayList<Command> cmd_list,String query_str) throws Exception
   {
    //System.out.println("Query string ["+query_str+"]");
    String[] param=new String[4];
    Object[] env=new Object[2];

    boolean completed=get_next_param(param,env,query_str);
    
    while(!completed)
     {
      Command c=new Command();

      c.query=query_str;
      
      //System.out.println("p0=["+param[0]+"]");
      
      if(param[0].equals("type"))       c.cmd_domain=CMD_GET;
      else if(param[0].equals("write")) c.cmd_domain=CMD_WRITE;
      else if(param[0].equals("meta"))  c.cmd_domain=CMD_META;
      else throw new Exception("Unsupported request type ["+param[0]+"]");
      
      c.obj_name=param[1];
      c.obj_version=param[2];
      c.method=param[3];
      if(c.method==null) c.method="DEFAULT";       
       
      if(get_next_param(param,env,query_str)) throw new Exception("command parse completed prematurely! param0: "+param[0]+", name: "+c.obj_name+", version: "+c.obj_version+", method: "+c.method+", query_str: "+query_str);
      
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

    // if the URL had a slash (PathInfo) instead of a question mark
    //   (QueryString) then it will be at the beginning of par; remove it
    if(par.startsWith("/"))par=par.substring(1);
    
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
    String ret="d="+domain_name[cmd_domain]+",on="+obj_name+",ov="+obj_version;
    ret+=",m="+method+",e="+encoder;
    return ret;
   }
 }
