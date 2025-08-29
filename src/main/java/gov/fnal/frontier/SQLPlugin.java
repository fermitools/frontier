/**
 * SQL interface plugin
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 */

package gov.fnal.frontier;

import gov.fnal.frontier.fdo.*;
import gov.fnal.frontier.plugin.*;
import gov.fnal.frontier.codec.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import java.sql.*;
import javax.sql.DataSource;
import java.util.Hashtable;
import java.util.HashSet;
import com.jcraft.jzlib.*;
import java.util.regex.*;
import java.util.Iterator;
import java.util.Date;

public class SQLPlugin implements FrontierPlugin
 {
  private static DataSource dataSource;
  private java.sql.Connection connection;

  private static Hashtable<HashSet<String>,SQLTimes> tableNames=new Hashtable<HashSet<String>,SQLTimes>();

  private HashSet<String> queryTableNames=null;

  private String query;
  private String queryLower;

  private static synchronized void getDataSource() throws Exception
   {
    Context initContext=new InitialContext();
    Context envContext=(Context)initContext.lookup("java:/comp/env");
    //System.out.println("Looking for ["+Frontier.getDsName()+"]");
    dataSource=(DataSource)envContext.lookup(Frontier.getDsName());
    if(dataSource==null) throw new Exception("DataSource ["+Frontier.getDsName()+"] not found");
   }

  public SQLPlugin(FrontierDataStream fds) throws Exception
   {
    if(dataSource==null)
      getDataSource();

    String param=fds.getString("p1");
    //System.out.println("Got param ["+param+"]");
    
    StringBuffer sb=new StringBuffer(param);
    int len=param.length();
    for(int i=0;i<len;i++)
     {
      char ch=param.charAt(i);
      switch(ch)
       {
        case '.': sb.setCharAt(i,'+'); break;
        case '-': sb.setCharAt(i,'/'); break;
        case '_': sb.setCharAt(i,'='); break;
        default:
       }
     }
    //sb.append('\n'); //needed with Base64.decode, not Base64Coder.decode
    param=sb.toString();
    //System.out.println("Pre param ["+param+"]");
    
    byte[] bascii=param.getBytes("US-ASCII");
    
    //byte[] bbin=Base64.decode(bascii,0,bascii.length);
    byte[] bbin=Base64Coder.decode(bascii);
    
    byte[] buffer=new byte[2048];    
    java.io.ByteArrayOutputStream baos=new java.io.ByteArrayOutputStream();
    java.io.ByteArrayInputStream bais=new java.io.ByteArrayInputStream(bbin);
    //Sun's GZIP/ZIP sucks...
    //java.util.zip.ZipInputStream gzis=new java.util.zip.ZipInputStream(bais);
    //Use JZip
    ZInputStream gzis=new ZInputStream(bais);

    while((len=gzis.read(buffer,0,buffer.length))>=0)
     {
      baos.write(buffer,0,len);
     }
    byte[] bsql=baos.toByteArray();    
    
    query=new String(bsql,"US-ASCII");
    queryLower=query.toLowerCase();
    Frontier.Log("SQL ["+query+"]");
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
   
  public void fp_acquire() throws Exception
   {
    connection=dataSource.getConnection();
   }
   
  public void fp_release() throws Exception
   {
    if(connection!=null)
     {
      connection.close();
      connection=null;
     }
   }
   
  public int fp_get(DbConnectionMgr mgr,Encoder enc,String method) throws Exception
   {
    if(!method.equals("DEFAULT")) throw new Exception("Unknown method "+method);

    if(connection==null)
      throw new Exception("No database connection available");
    
    if(queryLower.indexOf("drop ")>=0 ||
       queryLower.indexOf("delete ")>=0 ||
       queryLower.indexOf("insert ")>=0 ||
       queryLower.indexOf("alter ")>=0 ||
       queryLower.indexOf("create ")>=0) throw new Exception("Query cancelled");
    
    PreparedStatement stmt=null;
    ResultSet rs=null;
    int row_count=0;
    try
     {
      Frontier.Log("Executing DB query");
      if((query.indexOf(':')==-1)||(query.indexOf('?')==-1))
        stmt=connection.prepareStatement(query);
      else
       {
	// Have query with question marks signifying bind variables
	//  and values for the variables following the queries separated
	//  by colons.  Pass those as separate string parameters.
        String[] tokens=query.split(":");
	stmt=connection.prepareStatement(tokens[0]);
	for(int i=1;i<tokens.length;i++)
	  stmt.setString(i,tokens[i]);
       }
      stmt.setFetchSize(100); /* huge performance boost for small rows */
      			      /* causes much better row prefetching */
			      /* 1000 & 10000 are slightly faster but cause
			         executeQuery to abort with OutOfMemoryError
				 for some queries with larger rows. */
      long timestamp=(new Date()).getTime();
      rs=stmt.executeQuery();
      mgr.cancelKeepAlive();
      long timestamp2=(new Date()).getTime();
      Frontier.Log("DB query finished msecs="+(timestamp2-timestamp));
      timestamp=timestamp2;

      ResultSetMetaData rsmd=rs.getMetaData();
      int cnum=rsmd.getColumnCount();
      
      for(int i=1;i<=cnum;i++)
       {
        String n=rsmd.getColumnName(i);
        String t=rsmd.getColumnTypeName(i);
        if(t=="NUMBER")
         {
          // append column precision to column type name if non-zero
	  // this was requested by Luis Ramos at CERN
          int colPrec=rsmd.getPrecision(i);
          if(colPrec!=0)
            t+="("+colPrec+")";
	 }
        enc.writeString(n);
        enc.writeString(t);
       }
      enc.writeEOR();
            
      while(rs.next())
       {
	row_count++;
        for(int i=1;i<=cnum;i++)
         {
          String t=rsmd.getColumnTypeName(i);
	  if(t=="BLOB")
	   {
	    Blob blob=rs.getBlob(i);
	    //byte[] b=blob.getBytes((long)1,(int)blob.length());
	    //enc.writeBytes(b);
	    if(blob!=null)
	      enc.writeStream(blob.getBinaryStream(),(int)blob.length());
	    else
	      enc.writeString(null);
	   }
	  else
	   {
            String s=rs.getString(i);
            enc.writeString(s);
	   }
         }
        enc.writeEOR();
       }

      timestamp2=(new Date()).getTime();
      Frontier.Log("DB data transferred msecs="+(timestamp2-timestamp));
     }
    catch(Exception e)
     {
      if(queryTableNames!=null)
       {
        // prefix queryTableNames to the exception message
        Exception newe=new Exception("While querying "+queryTableNames.toString()+": "+e.getMessage().trim());
        newe.setStackTrace(e.getStackTrace());
        throw newe;
       }
      else
       throw e;
     }
    finally
     {
      enc.flush();
      if(rs!=null) try{rs.close();}catch(Exception e){}
      if(stmt!=null) try{stmt.close();}catch(Exception e){}
     }        
    return row_count;
   }

      
  public int fp_meta(Encoder enc,String method) throws Exception
   {
    throw new Exception("META methods are not supported by this object");
   }
   
   
  public int fp_write(Encoder enc,String method) throws Exception
   {
    throw new Exception("Not implemented");
   }

  protected static synchronized SQLTimes getSQLTimesObject(HashSet<String> qTableNames,boolean createIfNecessary)
   {
    SQLTimes times=tableNames.get(qTableNames);
    if((times==null)&&createIfNecessary)
     {
      times=new SQLTimes(qTableNames);
      tableNames.put(qTableNames,times);
     }
    return times;
   }

  private HashSet<String> getQueryTableNames(String str) 
   {
    // table name ends in either whitespace or comma or close-paren
    Pattern endtablepat=Pattern.compile("[\\s,)]");
    Matcher endmatcher=endtablepat.matcher(str);
    // Another table follows if there's a comma following the previous
    //  matched table name, optionally preceded by an alias.
    // The \\G[^\\s,]+ skips past the table name, and it's used because
    //  there's otherwise no way to anchor the regex to the beginning of
    //  the rest of the string (up-arrow matches beginning of whole string).
    Pattern anothertablepat=Pattern.compile("\\G[^\\s,)]+[\\s]*[^\\s,)]*,[\\s]*");
    Matcher anothermatcher=anothertablepat.matcher(str);

    HashSet<String> tablesSet=new QueryTableSet<String>();
    int starttablename=0;

    while(true)
     {
      if(!endmatcher.find(starttablename))
       {
        if(Frontier.getHighVerbosity())Frontier.Log("getQueryTableNames: no end match, adding name "+str.substring(starttablename));
        tablesSet.add(str.substring(starttablename));
	break;
       }
      int endtablename=endmatcher.start();
      if(Frontier.getHighVerbosity())Frontier.Log("getQueryTableNames: adding name "+str.substring(starttablename,endtablename));
      tablesSet.add(str.substring(starttablename,endtablename));
      if(!anothermatcher.find())
       {
        if(Frontier.getHighVerbosity())Frontier.Log("getQueryTableNames: no more names found");
	break;
       }
      starttablename=anothermatcher.end();
     }

    return tablesSet;
   }

  private long isAllTable(int startfrom) throws Exception 
   {    
    int numTables=queryTableNames.size();
   
    Iterator it=queryTableNames.iterator();
    String queryTableName=(String)it.next(); // The first table name

    boolean isAllTable=(queryTableName.indexOf('.')==-1);
    if(Frontier.getHighVerbosity())Frontier.Log("numTables: "+numTables+", queryTableName: "+queryTableName);

    if(!isAllTable)
      return 0;
      
    // one of the special system tables without a '.'
    // if it starts with ALL_ and has a OWNER='XXX' then use a
    //   special queryTableName of "XXX".ALL_TABLES
    int startowner=queryLower.indexOf(" owner",startfrom); // This assumes only one table
    if(startowner==-1)
      startowner=queryLower.indexOf(".owner",startfrom);
    if((startowner>startfrom)&&(queryLower.substring(startfrom,startfrom+4).equals("all_")))
     {
	startowner+=6;
	while(query.charAt(startowner)==' ')startowner++;
	if(query.charAt(startowner)!='=')
	  return -1; // syntax error
	startowner++;
	while(query.charAt(startowner)==' ')startowner++;
	if(query.charAt(startowner)!='\'')
	  return -1; // syntax error
	startowner++;
	int endowner=query.indexOf('\'',startowner);
	if(endowner==-1)
	  return -1; // syntax error
	if(startowner==endowner)
	  return -1; // empty owner
        // replace queryTableNames by resulting table name
        queryTableNames.clear();
        queryTableNames.add("\""+query.substring(startowner,endowner)+"\".ALL_TABLES");
     }
    else
     {
        // else querying the timestamp isn't going to work, do without timestamps
        Frontier.Log("don't know how to query timestamp for table "+queryTableName);
        return -1;
     }
     return 0;
   }

  /* 
    Parsing: handled by fp_cachedLastModified() and getQueryTableNames() 
    Queries are expected to be of these forms:
      'select ... from table_nameS ...' 
      'select ... from table_nameS'
      'select ... from (select ... from table_nameS ...'
      'select ... from (select ... from table_nameS)'
    where
      table_nameAndAlias='tableName [alias]'	
      table_nameS='tableNameAndAlias [, tableNameAndAlias [, tableNameAndAlias ...]]'
    Special case, handled by isAllTable(): 
     ALL_x ... OWNER ...
     Example:
      SELECT ... FROM ALL_TABLES WHERE OWNER='CMS_COND_FRONTIER'
  */
  public long fp_cachedLastModified() throws Exception
   {
    if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified()");

    if(Frontier.validate_last_modified_seconds<=0)
      return -1;

    // - Must start with 'select X'
    if(!queryLower.startsWith("select ")){
      if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() !'select '");
      return -1;
    }
    // - Must start with 'select ... from '
    int startfrom=queryLower.indexOf(" from ");
    if(startfrom==-1)
     {
      if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() startfrom==-1");
      return -1;
     }
    startfrom+=6;
    // May be" 'select ... from (select ..."
    if(queryLower.startsWith("(select",startfrom))
     {
      //nested select
      startfrom=queryLower.indexOf(" from ",startfrom+7);
      if(startfrom==-1)
       {
        if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() startfrom==-1");
	return -1;
       }
      startfrom+=6;
     }
    queryTableNames=getQueryTableNames(queryLower.substring(startfrom));

    if(isAllTable(startfrom)<0)
     {
      if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() isAllTable()<0");
      return -1;
     }

    SQLTimes times=getSQLTimesObject(queryTableNames,false);
    if(times==null)
     {
      if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() times==null");
      return 0;
     }
    long last_modified=times.getCachedLastModified();
    if(last_modified>0)
      Frontier.Log("using cached last-modified time of "+queryTableNames.toString());
    if(Frontier.getHighVerbosity())Frontier.Log("fp_cachedLastModified() last_modified: "+last_modified);
    return last_modified;
   }

  public long fp_getLastModified(long if_modified_since) throws Exception
   {
    if(queryTableNames==null)
      throw new Exception("SQLPlugin usage error -- fp_cachedLastModified must be called and return zero before calling fp_getLastModified");
    SQLTimes times=getSQLTimesObject(queryTableNames,true);
    Frontier.Log("getting last-modified time of "+queryTableNames.toString());
    long last_modified=times.getLastModified(connection);
    return last_modified;
   }
 }

