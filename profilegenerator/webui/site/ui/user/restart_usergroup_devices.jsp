<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDevices</title></head>
<body>

<pgs:restartUserGroupDevices  usergroupid='<%=request.getParameter("usergroupid")%>' />
<%
	response.sendRedirect( "list_all_usergroups.jsp" );
%>
</body>
</html>