<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>ListUsers</title></head>
<body>
<h1>Displays All Policy Profiles</h1>
<%-- Custom Tag to Display Group Hierarchies, in this case user groups --%>
<pgs:listItems orgid='<%=request.getParameter("orgid")%>' 
			   type="refConfigSet" stylesheet="listgroups.xslt" />
</body>
</html>
