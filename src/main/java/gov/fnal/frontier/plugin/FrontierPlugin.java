/**
 * Frontier plugin interface abstract class
 * $Id$
 * 
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier.plugin;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.*;

public interface FrontierPlugin
 {
  public String[] fp_getMethods();
  public MethodDesc fp_getMethodDesc(String name) throws Exception;
  public void fp_acquire() throws Exception;
  public void fp_release() throws Exception;
  public int fp_get(DbConnectionMgr mgr,Encoder enc,String method) throws Exception;
  public int fp_meta(Encoder enc,String method) throws Exception;
  public int fp_write(Encoder enc,String method) throws Exception;  
  public long fp_cachedLastModified() throws Exception;
  public long fp_getLastModified(long if_modified_since) throws Exception;
 }
