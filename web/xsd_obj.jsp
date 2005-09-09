<%@ page
    import="gov.fnal.frontier.*,java.util.*, java.sql.*, java.io.*"
    errorPage="xsdquery_error.jsp" 
    session="true"
%><%
Boolean b_test=(Boolean)session.getAttribute("user");
if(b_test==null)
 {
  response.sendRedirect("login.jsp");
  return;
 }

String obj_name=request.getParameter("n");
String obj_ver=request.getParameter("v");
if(obj_name==null || obj_ver==null)
 {
  response.sendRedirect("xsd_edit.jsp");
  return;
 }

obj_name=obj_name.trim();
obj_ver=obj_ver.trim();
 
DbConnectionMgr connMgr=null;
Connection con=null;
PreparedStatement stmt=null;
ResultSet rs=null;
String result="";
  
String cmd=request.getParameter("cmd");
if(!b_test.booleanValue()) cmd=null;
if(cmd!=null && cmd.compareTo("save")==0)
 {
  try
   { 
    String new_obj_name=request.getParameter("new_name").trim();
    String new_obj_ver=request.getParameter("new_version").trim();
    if(new_obj_name.length()<1 || new_obj_ver.length()<1)
     throw new Exception("Name and version can not be empty.");
        
    String s_xsd=request.getParameter("xsd_text");
    byte[] xsd=s_xsd.getBytes("ISO-8859-1");
    ByteArrayInputStream bains=new ByteArrayInputStream(xsd,0,xsd.length);
    connMgr=DbConnectionMgr.getDbConnectionMgr();
    //connMgr.initialize();
    con=connMgr.acquire();
    if(obj_name.compareTo(new_obj_name)!=0 || obj_ver.compareTo(new_obj_ver)!=0)
     {
      obj_name=new_obj_name;
      obj_ver=new_obj_ver;
      stmt=con.prepareStatement("INSERT INTO frontier_descriptors (name,version,xsd_type,xsd_data,create_date,create_user) VALUES (?,?,?,?,SYSDATE,?)");
      stmt.setString(1,obj_name);
      stmt.setString(2,obj_ver);
      stmt.setString(3,"xml");
      stmt.setBinaryStream(4,bains,xsd.length);
      stmt.setString(5,"admin");      
     }
    else
     {
      stmt=con.prepareStatement("update frontier_descriptors set xsd_data=?,update_date=SYSDATE,update_user='admin' where name=? and version=?");    
      stmt.setBinaryStream(1,bains,xsd.length);    
      stmt.setString(2,obj_name);
      stmt.setString(3,obj_ver);
     }
    stmt.execute();
    con.commit();
   }
  finally
   {
    if(stmt!=null) try{stmt.close();}catch(Exception e){}
    if(connMgr!=null) try{connMgr.release(con);}catch(Exception e){}
   }
 }
  
String p_do_edit=request.getParameter("edit");
if(!b_test.booleanValue()) p_do_edit=null;

if(cmd==null && p_do_edit!=null && p_do_edit.compareTo("2")==0)
 {
  // Delete
  try
   { 
    connMgr=DbConnectionMgr.getDbConnectionMgr();
    //connMgr.initialize();
    con=connMgr.acquire();
    stmt=con.prepareStatement("delete from frontier_descriptors where name=? and version=?");
    stmt.setString(1,obj_name);
    stmt.setString(2,obj_ver);
    System.out.println("Delete is disabled.");
    //stmt.execute();
    //con.commit();
   }
  finally
   {
    if(stmt!=null) try{stmt.close();}catch(Exception e){}
    if(connMgr!=null) try{connMgr.release(con);}catch(Exception e){}
   } 
  response.sendRedirect("xsd_edit.jsp");
  return;
 }

boolean do_edit=p_do_edit!=null && p_do_edit.compareTo("1")==0;
 
 
try
 { 
  connMgr=DbConnectionMgr.getDbConnectionMgr();
  //connMgr.initialize();
  con=connMgr.acquire();
  stmt=con.prepareStatement("select xsd_data from frontier_descriptors where name = ? and version = ? ");
  stmt.setString(1,obj_name);
  stmt.setString(2,obj_ver);
  rs=stmt.executeQuery();
  rs.next();
  Blob blob=rs.getBlob(1);
  long len=blob.length();
  byte[] b_data=blob.getBytes((long)1,(int)len);
  result=new String(b_data);
 }
finally
 {
  if(rs!=null) try{rs.close();}catch(Exception e){}
  if(stmt!=null) try{stmt.close();}catch(Exception e){}
  if(connMgr!=null) try{connMgr.release(con);}catch(Exception e){}
 }
%><html>
<head>
<title>XSD <%=obj_name%>:<%=obj_ver%></title>
</head>
<br>
<body>
<%=do_edit?"<b><font color=\"red\">&nbsp;&nbsp;&nbsp;Editor mode!</font></b>":""%>
<br>
<br>
<form name="frm0" action="xsd_obj.jsp" method="post">
<table>
 <tr>
  <td>Name:</td>
  <td><input type="text" name="new_name" size="60" value="<%=obj_name%>" <%=do_edit?"":"readonly"%> ></td>
 </tr>
 <tr>
  <td>Version:</td>
  <td><input type="text" name="new_version" size="10" value="<%=obj_ver%>" <%=do_edit?"":"readonly"%> ></td>
 </tr>
</table>
<br>
<br>
<textarea name="xsd_text" cols="100" rows="45" <%=do_edit?"":"readonly"%>>
<%=result%>
</textarea>
<input type="hidden" name="n" value="<%=obj_name%>" >
<input type="hidden" name="v" value="<%=obj_ver%>" >
<input type="hidden" name="cmd" value="save" >
</form>
<br>
<table>
 <tr>
  <td><input type="button" value="Back" onclick="javascript:location='xsd_edit.jsp';"></td>
  <td width="30">&nbsp;</td>
  <td>
   <%if(do_edit){%>
   <input type="button" value="Save" <%=b_test.booleanValue()?"":"disabled"%> onclick="javascript:document.frm0.submit();">   
   <%}else if(b_test.booleanValue()){%>
   <input type="button" value="Edit" <%=b_test.booleanValue()?"":"disabled"%> onclick="javascript:location='xsd_obj.jsp?n=<%=obj_name%>&v=<%=obj_ver%>&edit=1';">
   </td><td width="30">&nbsp;</td><td>
   <input type="button" value="Delete" disabled onclick="javascript:location='xsd_obj.jsp?n=<%=obj_name%>&v=<%=obj_ver%>&edit=2';">
   <%}%>
  </td>
 </tr>
</table>
</body>

