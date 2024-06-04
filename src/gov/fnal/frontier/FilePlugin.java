/**
 * File interface plugin
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

/**
 * Supports both regular files and files that come from http URLs:
 *  if FileBaseDirectory starts with "http://" then the files will be
 *  read from that "directory".  %xx in the given url will be converted
 *  to the corresponding ascii character.
 */

package gov.fnal.frontier;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import java.io.*;
import java.net.Socket;
import javax.net.ssl.*;
import javax.net.*;
import java.util.Date;
import java.util.concurrent.Semaphore;

public class FilePlugin implements FrontierPlugin
 {
  private String param;
  private BufferedInputStream instream;
  private File file;
  private int length;
  private boolean chunked;
  private boolean acquired;

  private static Semaphore semaphore;
  private static synchronized void initSemaphore() throws Exception
   {
    if(semaphore==null)
      semaphore=new Semaphore(Frontier.getMaxFileConnections(),true);
   }

  private String readHeaderLine(BufferedInputStream in) throws Exception
   {
    // Replacement for readLine because that requires BufferedReader which
    //  wants everything as chars instead of bytes and reads ahead beyond
    //  the header.  Ignore bytes beyond 256 in a header.  Ignore carriage
    //  return, and don't include the newline in the result.
    byte[] buf=new byte[256];
    int idx=0;
    int b;
    while((b=in.read())!=-1)
     {
      if(b=='\r')
	continue;
      if(b=='\n')
	return new String(buf,0,idx);
      if(idx<256)
	buf[idx++]=(byte)b;
     }
    return null;
   }

  public FilePlugin(FrontierDataStream fds) throws Exception
   {
    param=fds.getString("p1");
   }

  public void fp_acquire() throws Exception
   {
    if(semaphore==null)
      initSemaphore();
    // avoid doing too many of these in parallel
    semaphore.acquire();
    acquired=true;

   }

  public void fp_release() throws Exception
   {
    if(acquired)
     {
      semaphore.release();
      acquired=false;
     }
   }

  public String[] fp_getMethods()
   {
    String[] ret=new String[1];
    ret[0]="DEFAULT";
    return ret;
   }
   
   
  public MethodDesc fp_getMethodDesc(String name) throws Exception
   {
    MethodDesc ret=new MethodDesc("DEFAULT","get",false,((long)60*60*24*7*1000),"free","public");
    return ret;
   }
   
  public long fp_cachedLastModified() throws Exception
   {
    String baseDir=Frontier.getFileBaseDir();
    if(baseDir==null)
      throw new Exception("FileBaseDirectory not defined");

    if (baseDir.substring(0,7).equals("http://") || baseDir.substring(0,8).equals("https://"))
     {
      // We'll return an answer later in getLastModified, don't want to
      //  ask the server until we have acquired the connection
      return 0;
     }

    // For files, read the lastModified time before acquiring a
    //  connection so an immediate NOT MODIFIED can be returned if
    //  an If-Modified-Since was given and the file hasn't changed

    // eliminate the possibility of any "../" to escape the base path
    if(param.indexOf("..")>=0)
      throw new Exception("'..' not allowed in file path "+param);

    String pathname=baseDir+"/"+param;
    file=new File(pathname);
    length=(int)file.length();
    long lastModified=file.lastModified();

    if(!file.canRead())
     {
      Frontier.Log("Cannot read path "+pathname);
      throw new Exception("Cannot read file "+param);
     }

    // wait to actually open the file until after connection is acquired

    if(Frontier.getHighVerbosity())Frontier.Log("FilePlugin.fp_cachedLastModified returning "+lastModified);
    return lastModified;
   }

  private long open_connection(long if_modified_since) throws Exception
   {
    String baseDir=Frontier.getFileBaseDir();
    if(baseDir==null)
      throw new Exception("FileBaseDirectory not defined");

    long lastModified=0;
    if (baseDir.substring(0,7).equals("http://") || baseDir.substring(0,8).equals("https://"))
     {
      Integer index = 7;
      int port=80;
      String protocol = "http";
      if (baseDir.substring(0,8).equals("https://"))
       {
        index = 8;
        protocol = "https";
        port = 443;
       }
      // Retrieve file from http
      String basePath="/";
      String host=baseDir.substring(index);
      int endHost=host.indexOf(':');
      if(endHost<0)
	endHost=host.indexOf('/');
      if(endHost>0)
       {
	host=host.substring(0,endHost);
	if(baseDir.charAt(index+endHost)==':')
	 {
	  String portStr=baseDir.substring(index+endHost+1);
	  int endPort=portStr.indexOf('/');
	  if(endPort>0)
	   {
	    portStr=portStr.substring(0,endPort);
	    basePath=baseDir.substring(index+endHost+1+endPort);
	   }
	  port=Integer.parseInt(portStr);
	 }
	else
	 {
	  basePath=baseDir.substring(index+endHost);
	  }
       }

      String getStr=basePath+param;
      String url= protocol + "://" + host + ":" + port + getStr;
      Frontier.Log("Reading url "+url);

      Socket sock;
      if (protocol.equals("http"))
       {
        sock = new Socket(host, port);
       }
      else
       {
        SSLSocketFactory socketFactory = (SSLSocketFactory) SSLSocketFactory.getDefault();
        sock = (SSLSocket) socketFactory.createSocket(host, port);
       }
      try
       {
        long timestamp=(new Date()).getTime();
	PrintWriter out=new PrintWriter(sock.getOutputStream(),true);

	out.println("GET "+getStr+" HTTP/1.1\r");
	out.println("User-Agent: frontier\r");
	out.println("Host: "+host+"\r");
	if(if_modified_since>0)
	  out.println("If-Modified-Since: "+FrontierServlet.dateHeader(if_modified_since)+"\r");
	out.println("\r");

	instream=new BufferedInputStream(sock.getInputStream());
	String line=readHeaderLine(instream);
	if(line==null)
	  throw new Exception("empty response from "+url);
	if(!line.substring(0,7).equals("HTTP/1."))
	  throw new Exception("bad response "+line+" from "+url);
	String responsecode=line.substring(8,8+5);
	if(responsecode.equals(" 304 "))
	 {
	  if(Frontier.getHighVerbosity())Frontier.Log("NOT MODIFIED response");
	  sock.close();
	  return if_modified_since;
	 }
	if(!responsecode.equals(" 200 "))
	  throw new Exception("bad response code "+line+" from "+url);
        if(Frontier.getHighVerbosity())Frontier.Log("OK");

	chunked=false;
	while((line=readHeaderLine(instream))!=null)
	 {
	  // look at each header line
	  if(line.equals(""))
	    break;
	  int colonidx=line.indexOf(':');
	  if(colonidx<0)
	    continue;
	  String key=line.substring(0,colonidx).toLowerCase();
	  String val=line.substring(colonidx+2);
	  // Frontier.Log("header: "+key+": "+line.substring(colonidx+2));
	  if(key.equals("content-length"))
	   {
	    length=Integer.parseInt(val);
	    Frontier.Log("Content-Length: "+length);
	   }
	  else if(key.equals("transfer-encoding"))
	   {
	    if(val.equals("chunked"))
	     {
	      chunked=true;
	      if(Frontier.getHighVerbosity())Frontier.Log("Transfer-Encoding: chunked");
	     }
	   }
	  else if(key.equals("last-modified"))
	   {
	    if(Frontier.getHighVerbosity())Frontier.Log("received last-modified "+val);
	    lastModified=FrontierServlet.parseDateHeader(val);
	   }
	 }

        Frontier.Log("Data ready length="+length+" msecs="+((new Date()).getTime()-timestamp));
       }
      catch(Exception e)
       {
	// if any exception, close the socket & rethrow
	sock.close();
	throw e;
       }
     }
    else
     {
      Frontier.Log("Reading "+length+"-byte file ["+param+"]");
      
      instream=new BufferedInputStream(new FileInputStream(file));

      // don't need lastModified here, returned it earlier
     }

    if(Frontier.getHighVerbosity())Frontier.Log("FilePlugin.open_connection returning "+lastModified);
    return lastModified;
   }

  public long fp_getLastModified(long if_modified_since) throws Exception
   {
    if(Frontier.getHighVerbosity())Frontier.Log("FilePlugin.fp_getLastModified()");
    return(open_connection(if_modified_since));
   }

  public int fp_get(DbConnectionMgr mgr,Encoder enc,String method) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);

    if(instream==null)
     {
      // this means it was too late to read the Last-Modified time,
      //  so open the connection now and ignore Last-Modified
      open_connection(0);
     }

    // now data is fully ready, cancel keep alive timer
    mgr.cancelKeepAlive();
    
    try
     {
      if(chunked)
       {
	while(true)
	 {
	  String line=readHeaderLine(instream);
	  if(line==null)
	    throw new Exception("premature end to chunked encoding");
	  int semicolon=line.indexOf(";");
	  if (semicolon!=-1)
	    line=line.substring(0,semicolon);
	  if(line.length()==0)
	    throw new Exception("premature blank line in chunked encoding");
	  int size=Integer.parseInt(line,16);
	  if(size==0)
	   {
	    // No more chunks to come.  There may be trailers, ignore them.
	    // Tead until a blank line.
	    while(true)
	     {
	      line=readHeaderLine(instream);
	      if(line==null)
	       {
		if(Frontier.getHighVerbosity())Frontier.Log("no terminating blank line, ignoring");
		break;
	       }
	      if(line.length()==0)
		// normal terminator
		break;
	      if(Frontier.getHighVerbosity())Frontier.Log("ignoring trailer "+line);
	     }
	    break;
	   }
	  if(Frontier.getHighVerbosity())Frontier.Log("read chunk of "+size+" bytes");
	  enc.writeStream(instream,size); 

	  line=readHeaderLine(instream);
	  if(line==null)
	    throw new Exception("non-empty chunk had nothing following it");
	  if(line.length()!=0)
	   {
	    if(Frontier.getHighVerbosity())Frontier.Log("chunk followed by line of len "+line.length()+": \""+line +"\"");
	    throw new Exception("chunk not followed by empty line");
	   }
	 }
       }
      else if(length==0)
       {
	// No length known, and chunked encoding not available.
	// Read a buffer's worth at a time, because the frontier protocol
	//   requires a length at the beginning of each section.
	int len;
	byte[] buf=new byte[65536];
	InputStream bais=new ByteArrayInputStream(buf);
	while((len=instream.read(buf,0,65536))>0)
	 {
	  if(Frontier.getHighVerbosity())Frontier.Log("read "+len+" bytes");
	  enc.writeStream(bais,len); 
	  bais.reset();
	 }
       }
      else
       {
	// Read the full content-length
        enc.writeStream(instream,length); 
       }
      enc.writeEOR();
     }
    finally
     {
      instream.close();
     }        
    return 1;
   }


  public int fp_meta(Encoder enc,String method) throws Exception
   {
    throw new Exception("META methods are not supported by this object");
   }
   
   
  public int fp_write(Encoder enc,String method) throws Exception
   {
    throw new Exception("Not implemented");
   }

 }
