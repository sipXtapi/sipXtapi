<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
    <head>
        <title>CreateApplicationGroup</title>
        <script language="JavaScript">
        function reloadForm()
        {
            location = "/pds/ui/popup/form_add_appset.jsp";
        }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createApplicationGroup name= '<%=request.getParameter("name")%>' />
    </body>
</html>

