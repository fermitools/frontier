/**
* A Base64 Encoder/Decoder.
* $Id$
*
* <p>
* This class is used to encode and decode data in Base64 format as described in RFC 1521.
*
* <p>
* This is "Open Source" software and released under the <a href="http://www.gnu.org/licenses/lgpl.html">GNU/LGPL</a> license.<br>
* It is provided "as is" without warranty of any kind.<br>
* Copyright 2003: Christian d'Heureuse, Inventec Informatik AG, Switzerland.<br>
* Home page: <a href="http://www.source-code.biz">www.source-code.biz</a><br>
*
* <p>
* Version history:<br>
* 2003-07-22 Christian d'Heureuse (chdh): Module created.<br>
* 2005-08-11 chdh: Lincense changed from GPL to LGPL.<br>
* 2006-11-21 chdh:<br>
*  &nbsp; Method encode(String) renamed to encodeString(String).<br>
*  &nbsp; Method decode(String) renamed to decodeString(String).<br>
*  &nbsp; New method encode(byte[],int) added.<br>
*  &nbsp; New method decode(String) added.<br>
* 2007-02-23 Dave Dykstra dwd@fnal.gov adapted for frontier servlet:<br>
*/

package gov.fnal.frontier.codec;

public class Base64Coder {

// Mapping table from 6-bit nibbles to Base64 characters.
private static byte[]    map1 = new byte[64];
   static {
      int i=0;
      for (byte c='A'; c<='Z'; c++) map1[i++] = c;
      for (byte c='a'; c<='z'; c++) map1[i++] = c;
      for (byte c='0'; c<='9'; c++) map1[i++] = c;
      map1[i++] = '+'; map1[i++] = '/'; }

// Mapping table from Base64 characters to 6-bit nibbles.
private static byte[]    map2 = new byte[128];
   static {
      for (int i=0; i<map2.length; i++) map2[i] = -1;
      for (int i=0; i<64; i++) map2[map1[i]] = (byte)i; }

/**
* Encodes a string into Base64 format.
* No blanks or line breaks are inserted.
* @param s  a String to be encoded.
* @return   A String with the Base64 encoded data.
*/
public static String encodeString (String s) {
   return new String(encode(s.getBytes())); }

/**
* Encodes a byte array into Base64 format.
* No blanks or line breaks are inserted.
* @param in  an array containing the data bytes to be encoded.
* @return    A byte array with the Base64 encoded data.
*/
public static byte[] encode (byte[] in) {
   return encode(in,in.length); }

/**
* Encodes a byte array into Base64 format.
* No blanks or line breaks are inserted.
* @param in   an array containing the data bytes to be encoded.
* @param inoff number of bytes to process in <code>in</code>.
* @param iLen number of bytes to process in <code>in</code>.
* @return     A byte array with the Base64 encoded data.
*/
public static byte[] encode (byte[] in, int iLen) {
   int oLen = ((iLen+2)/3)*4;         // output length including padding
   byte[] out = new byte[oLen];
   encode(in, 0, iLen, out, 0);
   return out; }

/**
* Encodes byte array into given output buffer in Base64 format.
* @param in   an array containing the data bytes to be encoded.
* @param ip   starting offset in <code>in</code>
* @param iLen number of bytes to process in <code>in</code>.
* @param out  a byte array with at least ((iLen+2)/3)*4 bytes left
* @param op   starting offset in <code>out</code>
* @return     number of bytes encoded into <code>out</code> (including starting amount)
*/
public static int encode (byte[] in, int ip, int iLen, byte[] out, int op) {
   int oDataLen = op+(iLen*4+2)/3;       // output length without padding
   int ep = ip + iLen;
   while (ip < ep) {
      int i0 = in[ip++] & 0xff;
      int i1 = ip < ep ? in[ip++] & 0xff : 0;
      int i2 = ip < ep ? in[ip++] & 0xff : 0;
      int o0 = i0 >>> 2;
      int o1 = ((i0 &   3) << 4) | (i1 >>> 4);
      int o2 = ((i1 & 0xf) << 2) | (i2 >>> 6);
      int o3 = i2 & 0x3F;
      out[op++] = map1[o0];
      out[op++] = map1[o1];
      if (op < oDataLen) out[op++] = map1[o2]; else out[op++] = '=';
      if (op < oDataLen) out[op++] = map1[o3]; else out[op++] = '='; }
   return op; }

/**
* Decodes a string from Base64 format.
* @param s  a Base64 String to be decoded.
* @return   A String containing the decoded data.
* @throws   IllegalArgumentException if the input is not valid Base64 encoded data.
*/
//public static String decodeString (String s) {
//   return new String(decode(s)); }

/**
* Decodes a byte array from Base64 format.
* @param s  a Base64 String to be decoded.
* @return   An array containing the decoded data bytes.
* @throws   IllegalArgumentException if the input is not valid Base64 encoded data.
*/
//public static byte[] decode (String s) {
//   return decode(s.toCharArray()); }

/**
* Decodes a byte array from Base64 format.
* No blanks or line breaks are allowed within the Base64 encoded data.
* @param in  a byte array containing the Base64 encoded data.
* @return    An array containing the decoded data bytes.
* @throws    IllegalArgumentException if the input is not valid Base64 encoded data.
*/
public static byte[] decode (byte[] in) {
   int iLen = in.length;
   while (iLen > 0 && in[iLen-1] == '=') iLen--;
   int oLen = ((iLen*3)+2) / 4;
   byte[] out = new byte[oLen];
   int ip = 0;
   int op = 0;
   while (ip < iLen) {
      int i0 = in[ip++];
      int i1 = ip < iLen ? in[ip++] : 'A'; // 'A' translates to zero
      int i2 = ip < iLen ? in[ip++] : 'A';
      int i3 = ip < iLen ? in[ip++] : 'A';
      if ((i0 > 127)||(i1 > 127)||(i2 > 127)||(i3 > 127))
         throw new IllegalArgumentException ("Illegal character in Base64 encoded data i0 = "+i0+", i1 = "+i1+", i2 = "+i2+", i3 = "+i3);
      int b0 = map2[i0];
      int b1 = map2[i1];
      int b2 = map2[i2];
      int b3 = map2[i3];
      if ((b0 < 0)||(b1 < 0)||(b2 < 0)||(b3 < 0))
         throw new IllegalArgumentException ("Illegal character in Base64 encoded data i0 = "+i0+", i1 = "+i1+", i2 = "+i2+", i3 = "+i3);
      int o0 = ( b0       <<2) | (b1>>>4);
      int o1 = ((b1 & 0xf)<<4) | (b2>>>2);
      int o2 = ((b2 &   3)<<6) |  b3;
      out[op++] = (byte)o0;
      if (op<oLen) out[op++] = (byte)o1;
      if (op<oLen) out[op++] = (byte)o2; }
   return out; }

// Dummy constructor.
private Base64Coder() {}

} // end class Base64Coder
