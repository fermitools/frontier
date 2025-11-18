/**
 * Interface to Frontier Data Object. Any DataObject implementation
 * must implement this interface
 *
 * @author Server Kosyakov
 * @version $Id$
 * @see gov.fnal.frontier.PluginDataObject
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier.fdo;

import java.io.*;
import jakarta.servlet.ServletOutputStream;

public interface FrontierDataObject
 {
  public MethodDesc fdo_getMethodDesc(String method) throws Exception;
  public void fdo_start(ServletOutputStream os) throws Exception;
  public int fdo_get(Encoder enc,String method,ServletOutputStream os) throws Exception;
  public int fdo_meta(Encoder enc,String method) throws Exception;
  public int fdo_write(Encoder enc,String method,ServletOutputStream os) throws Exception;
  public long fdo_cachedLastModified() throws Exception;
  public long fdo_getLastModified(ServletOutputStream os,long if_modified_since) throws Exception;
  public void fdo_close(ServletOutputStream os) throws Exception;
 }
