<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>CopyUser</title></head>
<body>
<h1>Copies a User</h1>

<pgs:copyUser sourceuserid='<%=request.getParameter("sourceuserid")%>' />
<pgs:generateCredentials/>
<pgs:generateAliases/>
<pgs:generatePermissions/>
<pgs:generateExtensions/>
<%
	response.sendRedirect( "list_all_users.jsp" );
%>
</body>
</html>
