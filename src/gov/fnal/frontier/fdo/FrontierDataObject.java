package gov.fnal.frontier.fdo;

import java.io.*;
import gov.fnal.frontier.codec.Encoder;

/**
 * Interface to Frontier Data Object. Any DataObject implementation
 * must implement this interface
 *
 * @author Server Kosyakov
 * @version $Id$
 * @see gov.fnal.frontier.XsdDataObject
 * @see gov.fnal.frontier.Xsd2DataObject
 * @see gov.fnal.frontier.PluginDataObject
 */
public interface FrontierDataObject
 {
  public void fdo_init(byte[] body) throws Exception;
  public MethodDesc fdo_getMethodDesc(String method) throws Exception;
  public int fdo_get(Encoder enc,String method,FrontierDataStream fds) throws Exception;
  public int fdo_meta(Encoder enc,String method) throws Exception;
  public int fdo_write(Encoder enc,String method,FrontierDataStream fds) throws Exception;
 }
