<%@ page errorPage="xsdquery_error.jsp" import="gov.fnal.frontier.*,java.util.*" %>
<% ServicerFactory sf = new ServicerFactory();
   DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();
   connMgr.initialize();
   String type = request.getParameter("type");
   String version = request.getParameter("version");
%>
<html>
<body bgcolor="#cc6600">
<head>
<title>xsdquery_action.jpg</title>
</head>
<body>
    <TABLE WIDTH="100%">
      <TR>
        <TD WIDTH="90">
          <IMG SRC="frontier_left.png" ALT="{Cowboy}" BORDER="0" height="100">
          </TD>
        <TD WIDTH="100%" ALIGN="CENTER">
          <B><font SIZE="+3"><font color="#oo66cc">F</font>rontier</font></B>
            </TD>
        <TD WIDTH="100">
          <IMG SRC="frontier_right.png" ALT="{Cowboy}" BORDER="0" height="100">
        </TD>
      </TR>
    </TABLE>
<hr />
<table WIDTH="100%">
    <tr>
            <td WIDTH="100%" ALIGN="CENTER">
                    <B><font SIZE="+3"><font color="#oo66cc">X</font>ML
                                     <font color="#oo66cc">S</font>ervice
                                   <font color="#oo66cc">D</font>escriptor Details
                  </font></B>
            </td>
    </tr>
</table>
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
