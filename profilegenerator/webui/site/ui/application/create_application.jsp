<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
    <head>
    <script language="JavaScript">
        function reloadForm()
        {
            location = "/pds/ui/popup/form_add_application.jsp";
        }
    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    <title>CreateApplication</title>
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createApplication  name= '<%=request.getParameter("name")%>'
           devicetypeid= '<%=request.getParameter("devicetypeid")%>'
           refpropertyid= '<%=request.getParameter("refpropertyid")%>'
           url= '<%=request.getParameter("url")%>'
           description= '<%=request.getParameter("description")%>' />

    </body>
</html>