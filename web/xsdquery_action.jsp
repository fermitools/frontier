<%@ page errorPage="xsdquery_error.jsp" import="gov.fnal.frontier.*,java.util.*" %>
<% ServicerFactory sf = new ServicerFactory();
   DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();
   connMgr.initialize();
   String type = request.getParameter("type");
   String version = request.getParameter("version");
%>
<html>
<body bgcolor="yellow">
<head>
<title>xsdquery_action.jpg</title>
</head>
<body>
<H1 align="center"><font color="red">X</font>ML <font color="red">S</font>ervice<font color="red">D</font>escriptor Detail</h1>
<table align="center">
    <tr>
        <td>Table: <b><%= type %></b> Version: <b><%= version %></b></td>
    </tr>
</table>
<br /><br />
<table border="3" align="center">
<tr>
          <th>Attribute</th>
          <th>Data Type</th>
</tr>
 <% Servicer servicer = sf.load(type,version);
    ListIterator it = servicer.getAttributes();
    while (it.hasNext()) {
        Attribute at = (Attribute) it.next(); %>
        <tr>
            <td bgcolor="white"> <%= at.getField() %> </td>
            <td bgcolor="white"> <%= at.getType()  %> </td>
        </tr>
 <% } %>
</table>
</body>
</html>
