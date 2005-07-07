<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_devicegroups.jsp" );
%>
    <html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDeviceGroupDevices</title></head>
<body>

<pgs:restartDeviceGroupDevices  devicegroupid='<%=request.getParameter("devicegroupid")%>' />
</body>
</html>
