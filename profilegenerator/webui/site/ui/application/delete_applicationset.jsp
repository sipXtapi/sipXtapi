<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_applicationsets.jsp" );
%>
<html>
<head>
<title>DeleteApplicationSet</title></head>
<body>
<%-- Custom Tag to Delete an Application Set--%>
<pgs:deleteApplicationSet applicationsetid= '<%=request.getParameter("applicationsetid")%>' />
</body>
</html>