<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
<title>Get Database Version Information</title>
</head>
<body class="bglight">
<p></p>
<p class="formtext">Upgrading the database...</p>
<pgs:upgradeDatabase version= '<%= request.getParameter( "version" )%>' />
<%
    response.sendRedirect( "../administration/about.jsp" );
%>
</body>
</html>
