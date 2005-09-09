<%@ page
    import="gov.fnal.frontier.*,java.util.*, java.sql.*"
    errorPage="xsdquery_error.jsp" 
    session="true"
%><%
Boolean b_test=(Boolean)session.getAttribute("user");
if(b_test==null)
 {
  response.sendRedirect("login.jsp");
  return;
 }

DbConnectionMgr connMgr=null;
Connection con=null;
PreparedStatement stmt=null;
ResultSet rs=null;
ArrayList data=new ArrayList();
 
try
 { 
  connMgr=DbConnectionMgr.getDbConnectionMgr();
  //connMgr.initialize();
  con=connMgr.acquire();
  stmt=con.prepareStatement("select name, version from frontier_descriptors order by name,version");
  rs=stmt.executeQuery();
  while(rs.next())
   {
    String[] row=new String[2];
    row[0]=rs.getString("name").trim();
    row[1]=rs.getString("version").trim();
    data.add(row);
   }
 }
finally
 {
  if(rs!=null) try{rs.close();}catch(Exception e){}
  if(stmt!=null) try{stmt.close();}catch(Exception e){}
  if(connMgr!=null) try{connMgr.release(con);}catch(Exception e){}
 }
%><html>
<head>
<title>Objects list</title>
</head>
<body>
<table width="50%" border="1" bordercolor="black">
<%for(int i=0;i<data.size();i++){String[] row=(String[])data.get(i);
%><tr>
   <td><%=row[0]%></td>
   <td><%=row[1]%></td>
   <td align="center"><a href="xsd_obj.jsp?n=<%=row[0]%>&v=<%=row[1]%>">View</a>
  </tr>
<%}%>
</table>
</body>

