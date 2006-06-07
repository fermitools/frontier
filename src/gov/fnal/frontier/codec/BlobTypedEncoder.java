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
import java.util.zip.*;
import gov.fnal.frontier.fdo.Encoder;

/* this filter can be inserted to print out a datastream in hex */
class MyDebugOutputStream extends java.io.FilterOutputStream
 {
  private int cnt=0;
  public MyDebugOutputStream(OutputStream out)
   {
    super(out);
   }

  public void write(int b) throws IOException
   {
    if (cnt++ == 0) System.out.print("MyDebugOut: [");
    String hs = Integer.toHexString(b&0xff);
    if (hs.length() == 1) hs="0"+hs;
    System.out.println(hs);
    super.write(b);
   }

  public void flush() throws IOException
   {
    if (cnt > 0)
     {
      System.out.println("] "+cnt+" bytes written");
      cnt = 0;
     }
    super.flush();
   }
 }

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
  private DigestOutputStream dgos;
  private DeflaterOutputStream dfos;

  private int ziplevel=0;
  private long out_size=0;

  public BlobTypedEncoder(OutputStream out,String param) throws Exception
   {
    channel=out;

    baos=new ByteArrayOutputStream();
    b64os=new Base64.OutputStream(baos);
    md5=MessageDigest.getInstance("MD5");
    dgos=new DigestOutputStream(b64os,md5);

    if ((param.length() >= 3) && (param.substring(0,3).equals("zip")))
     {
      String zipparam=param.substring(3);
      if (zipparam.equals(""))
       {
	ziplevel=5;
       }
      else
       {
	Integer zipint;
	try {zipint=new Integer(zipparam);}
	catch(Exception e)
	 {
	  throw new Exception("BLOB zip level "+zipparam+" not an integer");
	 }
        ziplevel=zipint.intValue();
        if ((ziplevel < 0) || (ziplevel > 9))
	 {
          throw new Exception("BLOB zip level less than 0 or greater than 9");
	 }
       }
     }
    else if (param.length() != 0)
     throw new Exception("Unrecognized BLOB sub-encoding "+param);
    System.out.println("BLOB zip level ["+ziplevel+"]");

    if (ziplevel > 0)
     {
      dfos=new DeflaterOutputStream(dgos,new Deflater(ziplevel));
      os=new DataOutputStream(dfos);
     }
    else
     {
      dfos=null;
      os=new DataOutputStream(dgos);
     }
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
    if (dfos!=null) dfos.finish();
    b64os.flushBase64();
    dump();
   }


  public void close() throws Exception
   {
    flush();
    os.close();
    os=null;
    dfos=null;
    b64os=null;
   }
 }

