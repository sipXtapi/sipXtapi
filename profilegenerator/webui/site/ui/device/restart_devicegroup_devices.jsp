<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDeviceGroupDevices</title></head>
<body>

<pgs:restartDeviceGroupDevices  devicegroupid='<%=request.getParameter("devicegroupid")%>' />
<%
	response.sendRedirect( "list_all_devicegroups.jsp" );
%>
</body>
</html>