<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>DeleteApplicationSet</title></head>
<body>
<%-- Custom Tag to Delete an Application Set--%>
<pgs:deleteApplicationSet applicationsetid= '<%=request.getParameter("applicationsetid")%>' />
<%
	response.sendRedirect( "list_all_applicationsets.jsp" );
%>
</body>
</html>