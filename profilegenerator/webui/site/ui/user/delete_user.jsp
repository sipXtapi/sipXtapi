<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
	response.sendRedirect( "list_all_users.jsp?groupid=" + request.getParameter("grpid") );
%>
<html>
<head>
<title>DeleteUser</title></head>
<body>
<h1>Deletes a User</h1>
generateCredentials

<pgs:deleteUser userid='<%=request.getParameter("userid")%>' />
<pgs:generateCredentials/>
<pgs:generateAliases/>
<pgs:generatePermissions/>
<pgs:generateExtensions/>
<pgs:generateAuthExceptions changepointid='<%=request.getParameter("userid")%>'
    changepointtype="user"/>
</body>
</html>
