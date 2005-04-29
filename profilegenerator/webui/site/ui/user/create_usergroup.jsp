<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String parentGroupID = null;
    if ( !request.getParameter( "parentgroupid" ).equals("") )
    {
        parentGroupID = request.getParameter( "parentgroupid" );
    }
%>
<html>
    <head>
        <script language="JavaScript">
            function reloadForm()
            {
                location = "/pds/ui/popup/form_add_usergroup.jsp";
            }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <title>CreateDeviceGroup</title>
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createUserGroup
            usergroupname= '<%=request.getParameter("usergroupname").trim()%>'
            parentgroupid= '<%=parentGroupID%>'
            refconfigsetid= '<%=request.getParameter("refconfigsetid")%>'
            parameterstring= '<%="usergroupname=" + request.getParameter("usergroupname").trim() + "&" +
                                "parentgroupid=" + request.getParameter("parentgroupid") + "&" +
                                "refconfigsetid=" + request.getParameter("refconfigsetid") %>' />
    </body>
</html>
