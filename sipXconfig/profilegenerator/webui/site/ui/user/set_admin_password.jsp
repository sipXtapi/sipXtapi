<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Set Password</title></head>
<body>
<h1>Sets admin password - temporary only</h1>

<pgs:setAdminPassword   userid='<%=request.getParameter("userid")%>'
                        password='<%=request.getParameter("password")%>'
/>

</body>
</html>
