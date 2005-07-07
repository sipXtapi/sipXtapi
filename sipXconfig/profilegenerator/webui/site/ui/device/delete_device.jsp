<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_devices.jsp?groupid=" + request.getParameter("grpid") );
%>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>DeleteDevice</title></head>
<body class="bglight">
<h1 class="list">Delete a Device</h1>
<%-- Custom Tag to delete a Device --%>
<pgs:deleteDevice  deviceid= '<%=request.getParameter("deviceid")%>' />
<pgs:generateCredentials/>
</body>
</html>