package gov.fnal.frontier;

import java.io.*;
import gov.fnal.frontier.codec.Encoder;

public interface FrontierDataObject
 {
  public void fdo_init(byte[] body) throws Exception;
  public void fdo_get(OutputStream out,Encoder enc,String method,FrontierDataStream fds) throws Exception;
  public void fdo_meta(OutputStream out,Encoder enc) throws Exception;
 }
