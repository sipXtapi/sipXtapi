<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "../sipxchange/applications.jsp?userid="
        + request.getParameter("entityid"));
%>

<html>
<head>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
<title>ProjectProfiles</title>
</head>
<body class="bglight">
<div align="center">
    <p class="formtext">Sending Profiles...Please Wait</p>
</div>
<%-- The orgid parameter is optional, if it is not specified
	 the taglib will see it as null and look in the session object --%>

<pgs:projectProfiles    profiletypes ="5"
                        entitytype="user"
                        entityid= '<%=request.getParameter("entityid")%>'/>

</body>
</html>
