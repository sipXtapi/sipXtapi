<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Edit Extension Pool</title></head>
<body>
<pgs:editExtensionPool
    extensionpoolid='<%=request.getParameter("extensionpoolid")%>'
    name='<%=request.getParameter("name")%>' />
<pgs:generateCredentials/>
<pgs:generateAliases/>
<pgs:generatePermissions/>
<pgs:generateExtensions/>
<%
	response.sendRedirect( "extpool_details.jsp?extensionpoolid=" +
            request.getParameter("extensionpoolid"));
%>
</body>
</html>
