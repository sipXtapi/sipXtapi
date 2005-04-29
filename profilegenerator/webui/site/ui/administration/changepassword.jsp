<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
<head>
<script language="JavaScript">
    function closeWindow()
    {
        this.window.close();
    }
</script>
</head>
<body onload="closeWindow()">
<pgs:setAdminPassword userid='<%=request.getParameter("userid")%>'
	password='<%=request.getParameter("password")%>'/>

    <pgs:generateCredentials/>
</body>
</html>
