/**
 * File interface plugin
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import java.sql.Connection;
import java.io.File;
import java.io.FileInputStream;

public class FilePlugin implements FrontierPlugin
 {
  private String filename;
  private String pathname;
  private File file;

  public FilePlugin(FrontierDataStream fds) throws Exception
   {
    filename=fds.getString("p1");

    // eliminate the possibility of any "../" to escape the base path
    if(filename.indexOf("..")>0)
      throw new Exception("'..' not allowed in file path "+filename);

    if(Frontier.getFileBaseDir()==null)
      throw new Exception("FileBaseDirectory not defined");

    pathname=Frontier.getFileBaseDir()+"/"+filename;
    file=new File(pathname);
   }

  public String[] fp_getMethods()
   {
    String[] ret=new String[1];
    ret[0]="DEFAULT";
    return ret;
   }
   
   
  public MethodDesc fp_getMethodDesc(String name) throws Exception
   {
    MethodDesc ret=new MethodDesc("DEFAULT","get",false,((long)60*60*24*7*1000),"free","public");
    return ret;
   }
   
  public int fp_get(java.sql.Connection con,DbConnectionMgr mgr,Encoder enc,String method) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);
    
    Frontier.Log("Reading file ["+filename+"]");
    
    if(!file.canRead())
     {
      Frontier.Log("Cannot read path "+pathname);
      throw new Exception("Cannot read file "+filename);
     }

    FileInputStream fis=new FileInputStream(file);
    try
     {
      enc.writeStream(fis,(int)file.length()); 
     }
    finally
     {
      fis.close();
     }        
    return 1;
   }

      
  public int fp_meta(Encoder enc,String method) throws Exception
   {
    throw new Exception("META methods are not supported by this object");
   }
   
   
  public int fp_write(java.sql.Connection con,Encoder enc,String method) throws Exception
   {
    throw new Exception("Not implemented");
   }

  public long fp_cachedLastModified() throws Exception
   {
    return file.lastModified();
   }

  public long fp_getLastModified(java.sql.Connection con) throws Exception
   {
    // never called because fp_cachedLastModified() always returns an answer
    throw new Exception("Never called");
   }

 }
