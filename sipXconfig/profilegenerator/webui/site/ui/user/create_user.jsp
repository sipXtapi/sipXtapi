<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
    <head>
        <script language="JavaScript">
            function reloadForm()
            {
                location = "/pds/ui/popup/form_add_user.jsp";
            }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <title>CreateUser</title>
    </head>
    <body class="bglight" onload="reloadForm()" >
        <pgs:createUser firstname='<%=request.getParameter("firstname").trim()%>'
		lastname='<%=request.getParameter("lastname").trim()%>'
		password='<%=request.getParameter("password").trim()%>'
		refconfigsetid='<%=request.getParameter("refconfigsetid")%>'
		usergroupid='<%=request.getParameter("usergroupid")%>'
		displayid='<%=request.getParameter("displayid").trim()%>'
                extensionnumber='<%=request.getParameter("extensionnumber")%>'

        parameterstring='<%="firstname=" + request.getParameter("firstname").trim() + "&" +
                "lastname=" + request.getParameter("lastname").trim() + "&" +
                "password=" + request.getParameter("password").trim() + "&" +
                "refconfigsetid=" + request.getParameter("refconfigsetid") + "&" +
                "usergroupid=" + request.getParameter("usergroupid") + "&" +
                "displayid=" + request.getParameter("displayid").trim() + "&" +
                "extensionnumber=" + request.getParameter("extensionnumber")%>' />

        <pgs:generateCredentials/>
        <pgs:generatePermissions/>
        <pgs:generateExtensions/>
        <pgs:generateAliases/>
    </body>
</html>
