<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>

<title>Administrator Details</title>
</head>
<body>
<h1>ListAdministrators</h1>
<pgs:administratorDetails   stylesheet="listadministrators.xslt"
                            outputxml="true"
                            userid= '<%=request.getParameter("userid")%>'/>
</body>
</html>
