package gov.fnal.frontier;

import java.io.*;

public interface FrontierDataObject
 {
  void fdo_init(byte[] body) throws Exception;
  void fdo_get(OutputStream out,Encoder enc,String method,FrontierDataStream fds) throws Exception;
  void fdo_meta(OutputStream out,Encoder enc) throws Exception;
 }
