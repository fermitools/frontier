<%@ page 
    errorPage="xsdquery_error.jsp" 
    session="true"
%><%
String uname=request.getParameter("uname");
String passwd=request.getParameter("password");
boolean loginOk=false;
String msg="";
if(uname!=null && passwd!=null)
 {
  if(uname.compareTo("admin")==0 && passwd.compareTo("FrontCmS_09283")==0)
   {
    session.setAttribute("user",new Boolean(true));
    loginOk=true;
   }
  else if(uname.compareTo("frontier_reader")==0 && passwd.compareTo("reader")==0)
   {
    session.setAttribute("user",new Boolean(false));
    loginOk=true;
   }  
  else
   {
    msg="Login information is incorrect";
   }
 }
 
%><html>
<head> 
<title>XSD Admin Login</title>
</head>
<%
if(loginOk)
 {
%>
<body>
<script language="JavaScript">
window.location="xsd_edit.jsp";
</script>
</body>
</html>
<%
  return;
 }
%>
<body bgcolor="#00004f">
    <TABLE WIDTH="100%">
      <TR>
        <TD WIDTH="90">
          <IMG SRC="frontier_left.png" ALT="{Cowboy}" BORDER="0" height="100">
          </TD>
        <TD WIDTH="100%" ALIGN="CENTER">
          <B><font SIZE="+3" color="white">Frontier</font></B>
            </TD>
        <TD WIDTH="100">
          <IMG SRC="frontier_right.png" ALT="{Cowboy}" BORDER="0" height="100">
        </TD>
      </TR>
    </TABLE>
<hr />
<form name="frm0" action="login.jsp" method="post">
<table WIDTH="100%">
 <tr>
  <td align="center" colspan="2"><font size="+1" color="white"><%=msg%></font></td>
 </tr> 
 <tr>
  <td ALIGN="right"><font color="white">Username:</font></td>
  <td align="left"><input type="text" name="uname" value=""></td>
 </tr>
 <tr>
  <td ALIGN="right"><font color="white">Password:</font></td>
  <td align="left"><input type="password" name="password" value=""></td>
 </tr>
 <tr>
  <td align="center" colspan="2"><input type="submit" value="Submit"></td>
 </tr>
</table>
</form>
</body>
</html>
