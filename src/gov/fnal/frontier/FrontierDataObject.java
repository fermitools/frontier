package gov.fnal.frontier;

import java.io.*;
import gov.fnal.frontier.codec.Encoder;

public interface FrontierDataObject
 {
  public void fdo_init(byte[] body) throws Exception;
  public long fdo_get_expiration_time();
  public boolean fdo_is_no_cache();
  public int fdo_get(Encoder enc,String method,FrontierDataStream fds) throws Exception;
  public int fdo_meta(Encoder enc) throws Exception;
 }
