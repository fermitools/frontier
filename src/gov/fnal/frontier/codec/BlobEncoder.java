/**
 * Implementation of BLOB encoder (Base64 encoded binary stream)
 * as in 3.2.2.1 with updates (see ntier mail list)
 *
 * @author Sergey Kosyakov
 */
package gov.fnal.frontier.codec;

import java.io.*;

public class BlobEncoder implements Encoder
 {
  private static final int BUFFER_SIZE=16384;

  private DataOutputStream os;
  private Base64.OutputStream b64os;
  private ByteArrayOutputStream baos;
  private OutputStream channel;

  private long out_size=0;

  public BlobEncoder(OutputStream out) throws Exception
   {
    channel=out;

    baos=new ByteArrayOutputStream();
    b64os=new Base64.OutputStream(baos);
    os=new DataOutputStream(b64os);
   }


  private void dump() throws Exception
   {
    System.out.println("dump()");
    baos.writeTo(channel);
    baos.reset();
   }


  public void writeInt(int v) throws Exception
   {
    os.writeInt(v);
    out_size+=4;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeLong(long v) throws Exception
   {
    os.writeLong(v);
    out_size+=8;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeDouble(double v) throws Exception
   {
    os.writeDouble(v);
    out_size+=8;
    if(baos.size()>=BUFFER_SIZE) dump();
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


  public void flush() throws Exception
   {
    os.flush();
    b64os.flushBase64();
    dump();
   }


  public void close() throws Exception
   {
    flush();
    os.close();
    os=null;
    b64os=null;
   }
 }

