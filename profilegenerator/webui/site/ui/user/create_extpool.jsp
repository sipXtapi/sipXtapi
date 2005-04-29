<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String orgid = session.getAttribute("orgid").toString();
%>
<html>
    <head>
    <script language="JavaScript">
            function reloadForm()
            {
                location = "/pds/ui/popup/add_extpool.jsp";
            }
        </script>
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createExtensionPool
            organizationid= '<%=orgid%>'
            name= '<%=request.getParameter("name")%>' />
    </body>
</html>
