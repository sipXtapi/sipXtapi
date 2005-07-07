<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Get Database Version Information</title>
</head>
<body>
<h1>Database Version Info</h1>
<pgs:databaseVersion stylesheet="foo.xslt" outputxml="true" />
</body>
</html>
