<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Delete DeviceGroup</title></head>
<body>
<h1>Deletes a DeviceGroup</h1>

<pgs:deleteDeviceGroup
    devicegroupid= '<%=request.getParameter("devicegroupid")%>'
	 	option="0" />

<%
   response.sendRedirect( "list_all_devicegroups.jsp" );
%>
</body>
</html>