/**
 * Implementation of BLOB encoder (Base64 encoded binary stream)
 * which provides carrying types information.
 * $Id$
 *
 * @author Sergey Kosyakov
 * 
 *
 * Changes log
 * 08/18/2004 
 *   if String, byte[] or Date is null, set the NULL bit in the type info
 *   writeEOR() added, makes demarshalling more flexible
 */
package gov.fnal.frontier.codec;

import java.io.*;
import java.security.*;


public class BlobTypedEncoder implements Encoder
 {
  public static final byte BIT_NULL=(byte)(1<<7); // Signals that the value is NULL
  public static final byte TYPE_BYTE=0;
  public static final byte TYPE_INT4=1;
  public static final byte TYPE_INT8=2;
  public static final byte TYPE_FLOAT=3;
  public static final byte TYPE_DOUBLE=4;
  public static final byte TYPE_TIME=5;
  public static final byte TYPE_ARRAY_BYTE=6;
  public static final byte TYPE_EOR=7;   // End Of Record

  private static final int BUFFER_SIZE=16384;

  private DataOutputStream os;
  private MessageDigest md5;
  private Base64.OutputStream b64os;
  private ByteArrayOutputStream baos;
  private OutputStream channel;

  private long out_size=0;

  public BlobTypedEncoder(OutputStream out) throws Exception
   {
    channel=out;

    baos=new ByteArrayOutputStream();
    b64os=new Base64.OutputStream(baos);
    md5=MessageDigest.getInstance("MD5");
    os=new DataOutputStream(new DigestOutputStream(b64os,md5));
   }


  private void dump() throws Exception
   {
    //System.out.println("dump()");
    baos.writeTo(channel);
    baos.reset();
   }
   
   
  public void writeEOR() throws Exception
   {
    os.writeByte(TYPE_EOR);
    out_size+=1;
    if(baos.size()>=BUFFER_SIZE) dump();
   }


  public void writeInt(int v) throws Exception
   {
    os.writeByte(TYPE_INT4);
    os.writeInt(v);
    out_size+=5;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeLong(long v) throws Exception
   {
    os.writeByte(TYPE_INT8);
    os.writeLong(v);
    out_size+=9;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeDouble(double v) throws Exception
   {
    os.writeByte(TYPE_DOUBLE);
    os.writeDouble(v);
    out_size+=9;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeFloat(float v) throws Exception
   {
    os.writeByte(TYPE_FLOAT);
    os.writeFloat(v);
    out_size+=5;
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeString(String v) throws Exception
   {
    if(v==null)
     {
      os.writeByte(TYPE_ARRAY_BYTE | BIT_NULL);
      out_size+=1;
     }
    else
     {
      os.writeByte(TYPE_ARRAY_BYTE);
      os.writeInt(v.length());
      out_size+=5;
      os.writeBytes(v);
      out_size+=v.length();
     }
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeBytes(byte[] v) throws Exception
   {
    if(v==null)
     {
      os.writeByte(TYPE_ARRAY_BYTE | BIT_NULL);
      out_size+=1;
     }
    else
     {
      //System.out.println("writeBytes(): length "+v.length);
      os.writeByte(TYPE_ARRAY_BYTE);
      os.writeInt(v.length);
      out_size+=5;
      os.write(v,0,v.length);
      out_size+=v.length;
     }
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public void writeDate(java.util.Date v) throws Exception
   {
    if(v==null)
     {
      os.writeByte(TYPE_TIME | BIT_NULL);
      out_size+=1;
     }
    else
     {
      os.writeByte(TYPE_TIME);
      os.writeLong(v.getTime());
      out_size+=9;
     }
    if(baos.size()>=BUFFER_SIZE) dump();
   }

  public long getOutputSize()
   {
    return out_size;
   }

  public byte[] getMD5Digest() throws Exception
   {
    return md5.digest();
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

