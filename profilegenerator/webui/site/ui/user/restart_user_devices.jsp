<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_users.jsp?groupid=" + request.getParameter("grpid") );
%>
    <html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>RestartDevices</title></head>
<body>

<pgs:restartUserDevices  userid='<%=request.getParameter("userid")%>' />
</body>
</html>