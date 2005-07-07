<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>Get Database Version Information</title>
</head>
<body>
<h1>Reset patch status</h1>

<pgs:resetPatchStatus patchstatuslist= '<%= request.getParameter( "patchstatuslist" )%>' />

</body>
</html>
