package gov.fnal.frontier.codec;

import java.io.*;

public class DataEncBlobStream implements DataEncStream
 {
  protected DataOutputStream os;
  private long out_size=0;

  public DataEncBlobStream(OutputStream out) throws Exception
   {
    OutputStream b64os=new Base64.OutputStream(out);
    os=new DataOutputStream(b64os);
   }

  public void writeInt(int v) throws Exception
   {
    os.writeInt(v);
    out_size+=4;
   }

  public void writeLong(long v) throws Exception
   {
    os.writeLong(v);
    out_size+=8;
   }

  public void writeDouble(double v) throws Exception
   {
    os.writeDouble(v);
    out_size+=8;
   }

  public void writeString(String v) throws Exception
   {
    throw new Exception("Not implemented yet");
   }

  public void writeBytes(byte[] v) throws Exception
   {
    throw new Exception("Not implemented yet");
   }

  public void writeDate(java.sql.Date v) throws Exception
   {
    throw new Exception("Not implemented yet");
   }

  public long getOutputSize()
   {
    return out_size;
   }

  public byte[] getMD5Digest() throws Exception
   {
    throw new Exception("Not implemented yet");
   }

  public void close() throws Exception
   {
    os.flush();
   }
 }

