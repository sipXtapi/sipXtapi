<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Delete UserGroup</title></head>
<body>
<h1>Deletes a UserGroup</h1>

<pgs:deleteUserGroup usergroupid='<%=request.getParameter("usergroupid")%>'
			    option="0" />

<pgs:generateCredentials/>
<pgs:generateAliases/>
<pgs:generatePermissions/>
<pgs:generateExtensions/>
<pgs:generateAuthExceptions
    changepointid='<%=request.getParameter("usergroupid")%>'
    changepointtype="usergroup"/>

<%
	response.sendRedirect( "list_all_usergroups.jsp" );
%>
</body>
</html>