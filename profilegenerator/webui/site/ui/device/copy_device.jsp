<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>DeleteDevice</title></head>
<body class="bglight">
<h1 class="list">Copies a Device</h1>

<pgs:copyDevice  sourcedeviceid= '<%=request.getParameter("sourcedeviceid")%>' />
<pgs:generateCredentials/>
<%
   response.sendRedirect( "list_all_devices.jsp" );
%>
</body>
</html>