/**
 * Frontier encoder interface
 * @author: Sergey Kosyakov
 */
package gov.fnal.frontier.codec;

public interface Encoder
 {
  public void flush() throws Exception;
  public void writeInt(int v) throws Exception;
  public void writeLong(long v) throws Exception;
  public void writeDouble(double v) throws Exception;
  public void writeString(String v) throws Exception;
  public void writeBytes(byte[] v) throws Exception;
  public void writeDate(java.util.Date v) throws Exception;
  public long getOutputSize();
  public byte[] getMD5Digest() throws Exception;
  public void close() throws Exception;
 }

