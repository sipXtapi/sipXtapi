<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Log Success</title></head>
<body>
<h1></h1>

<pgs:logTXSuccess   userid= '<%= (String)session.getAttribute( "user" ) %>'
                    orgname= '<%= (String)session.getAttribute( "orgname" ) %>'
                    message= '<%= request.getParameter("message")%>' />
<%
   //response.sendRedirect( "" );
%>
</body>
</html>