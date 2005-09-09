<%@ page
    session="false"
%><%!
static final String data_line=
"0123456789"+
"0123456789"+
"0123456789"+
"0123456789"+
"0123456789"+
"0123456789"+
"0123456789"+
"0123456789"+
"\n";
%><%
String param=request.getParameter("size"); // Size in MB

System.out.println("Request fo size "+param+"MB");

response.setContentType("text/plain");
response.setCharacterEncoding("US-ASCII");

long exp_time=java.lang.System.currentTimeMillis();
exp_time=exp_time+((long)1000*60*60*24*30);
response.setDateHeader("Expires",exp_time);

long size=Long.parseLong(param)*1024*1024;

long repeat=size/data_line.length();
int  pad=(int)(size%data_line.length())-1;

int line_len=0;

while(repeat>0)
 {
%><%=data_line%><%
  --repeat;
 }

for(int i=0;i<pad;i++)
 {%><%='A'%><%}

System.out.println("Request end."); 
%>
