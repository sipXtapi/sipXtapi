<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_usergroups.jsp" );
%>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDevices</title></head>
<body>

<pgs:restartUserGroupDevices  usergroupid='<%=request.getParameter("usergroupid")%>' />
</body>
</html>
