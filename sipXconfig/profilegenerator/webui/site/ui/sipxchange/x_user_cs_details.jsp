<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>UserConfigSetDetails</title>
</head>
<body>
<form name="form1">
<pgs:x_userConfigurationSetDetails
                    stylesheet='<%= request.getParameter("stylesheet")%>'
                    outputxml="true"
                    userid='<%= request.getParameter("userid")%>'
                    refpropertycodes='<%= request.getParameter("refpropertycodes")%>'
                    profiletype='<%= request.getParameter("profiletype")%>'/>

</form>
</body>
</html>
