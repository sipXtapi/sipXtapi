<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Copy UserGroup</title></head>
<body>
<h1>Copies a UserGroup</h1>
<pgs:copyUserGroup sourceusergroupid='<%=request.getParameter("sourceusergroupid")%>' />
<%
	response.sendRedirect( "list_all_usergroups.jsp" );
%>
</body>
</html>