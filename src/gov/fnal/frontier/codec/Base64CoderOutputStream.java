/**
 * OutputStream for (slightly modified) public domain Base64Coder.
 * $Id$
 *
 * @author Dave Dykstra dwd@fnal.gov
 * 
 */

package gov.fnal.frontier.codec;

import java.io.*;

class Base64CoderOutputStream extends java.io.FilterOutputStream
 {
  private byte[] savebytes=new byte[3];
  private int nsavedbytes=0;
  private byte[] outbuf=new byte[76];
  private int noutbytes=0;
  private byte[] onebyte=new byte[1];

  public Base64CoderOutputStream(OutputStream out)
   {
    super(out);
   }

  private void endline(boolean isFinal) throws IOException
   {
    out.write(outbuf,0,noutbytes);
    if(!isFinal)
      out.write('\n');
    noutbytes=0;
   }

  public void write(int b) throws IOException
   {
    onebyte[0]=(byte)b;
    write(onebyte,0,1);
   }

  public void write(byte[] b, int off, int len) throws IOException
   {
    if(len==0)return;
    if(nsavedbytes>0)
     {
      /* add 1 or 2 bytes to leftover to make a new 3-byte group, encode it */
      savebytes[nsavedbytes++]=b[off++];
      --len;
      if((len>0)&&(nsavedbytes<3))
       {
        savebytes[nsavedbytes++]=b[off++];
        --len;
       }
      if(nsavedbytes<3)return;  /* needed 2 bytes, only got one */
      noutbytes=Base64Coder.encode(savebytes,0,nsavedbytes,outbuf,noutbytes);
      nsavedbytes=0;
      if(noutbytes>=76) endline(false);
      if(len==0)return;
     }
     /* write out as many full 76-byte lines as available */
     /* note that noutbytes will always be a multiple of 4 */
     int maxwrite;
     while((maxwrite=(((76-noutbytes)/4)*3))<=len)
      {
       noutbytes=Base64Coder.encode(b,off,maxwrite,outbuf,noutbytes);
       len-=maxwrite;
       off+=maxwrite;
       endline(false);
      }
     /* write out amount left that's full 3-byte groups*/
     maxwrite=(len/3)*3;
     noutbytes=Base64Coder.encode(b,off,maxwrite,outbuf,noutbytes);
     len-=maxwrite;
     off+=maxwrite;
     /* save leftover 1 or 2 bytes if there are any */
     if(len>0)
      {
       savebytes[nsavedbytes++]=b[off++];
       --len;
       if(len>0)
        {
         savebytes[nsavedbytes++]=b[off++];
	 --len;
	}
      }
   }

  public void flushBase64() throws IOException
   {
    if(nsavedbytes>0)
     {
      noutbytes=Base64Coder.encode(savebytes,0,nsavedbytes,outbuf,noutbytes);
      nsavedbytes=0;
     }
    endline(true);
    out.flush();
   }
 }

