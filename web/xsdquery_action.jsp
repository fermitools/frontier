<%@ page errorPage="xsdquery_error.jsp" import="gov.fnal.frontier.*,java.util.*" %>
<% ServicerFactory sf = new ServicerFactory();
   DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();
   connMgr.initialize();
   String type = request.getParameter("type");
   String version = request.getParameter("version");
%>
<html>
<head>
<title>spw action</title>
</head>
<body>
<H1 align="center">XML Service Descriptor for </h1>
<table align="center">
    <tr>
        <td>Table: <b><%= type %></b></td>
    </tr>
    <tr>
        <td>Version: <b><%= version %></b></td>
    </tr>
</table>
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
            <td> <%= at.getField() %> </td>
            <td> <%= at.getType()  %> </td>
        </tr>
 <% } %>
</table>
</body>
</html>
