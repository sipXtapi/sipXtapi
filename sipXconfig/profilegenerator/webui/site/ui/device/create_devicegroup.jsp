<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String parentGroupID = null;
    if ( !request.getParameter( "parentgroupid" ).equals("") &&
        request.getParameter( "parentgroupid" ) != null )
    {
        parentGroupID = request.getParameter( "parentgroupid" );
    }
%>
<html>
    <head>
        <script language="JavaScript">
            function reloadForm()
            {
                location = "/pds/ui/popup/form_add_devicegroup.jsp";
            }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <title>CreateDeviceGroup</title>
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createDeviceGroup
            groupname= '<%=request.getParameter("groupname").trim()%>'
            parentgroupid= '<%=parentGroupID%>'
            refconfigsetid= '<%=request.getParameter("refconfigsetid")%>'
            parameterstring=
                '<%="groupname=" + request.getParameter("groupname").trim() + "&" +
                "parentgroupid=" + parentGroupID + "&" +
                "refconfigsetid=" + request.getParameter("refconfigsetid")%>' />

    </body>
</html>