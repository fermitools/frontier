/*
 * BLOB encoder test
 *
 * @author Sergey Kosyakov
 */
package gov.fnal.frontier.codec;

import java.io.*;

public class test_be
 {
  public static void main(String[] argv)
   {
    try
     {
      do_it();
     }
    catch(Exception e)
     {
      System.out.println("Error: "+e);
      e.printStackTrace();
     }
   }



  public static void do_it() throws Exception
   {
    ByteArrayOutputStream baos=new ByteArrayOutputStream();
    BlobEncoder be=new BlobEncoder((OutputStream)baos);
    be.writeInt(1);
    be.flush();
    be.writeInt(1);
    be.flush();
    for(int i=0;i<100;i++) be.writeInt(i);
    be.close();
    String out=baos.toString();
    System.out.println("Out <"+out+">");
   }
 }

