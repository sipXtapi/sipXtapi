<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_devices.jsp?groupid=" + request.getParameter("grpid") );
%>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDevice</title></head>
<body>

<pgs:restartDevice  deviceid='<%=request.getParameter("deviceid")%>' />
</body>
</html>
