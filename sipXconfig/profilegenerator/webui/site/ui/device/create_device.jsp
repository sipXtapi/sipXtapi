<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String csd = request.getParameter( "devicetypeid" );
%>
<html>
    <head>
        <script language="JavaScript">
            function reloadForm()
            {
                location = "/pds/ui/popup/form_add_device.jsp";
            }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <title>CreateDevice</title>
    </head>
    <body class="bglight" onload="reloadForm()">
        <pgs:createDevice  coresoftwaredetailsid= '<%=csd%>'
           description= '<%=request.getParameter("description").trim()%>'
           devicegroupid= '<%=request.getParameter("devicegroupid").trim()%>'
           refconfigsetid= '<%=request.getParameter("refconfigsetid").trim()%>'
           shortname= '<%=request.getParameter("shortname").trim()%>'
           serialnumber= '<%=request.getParameter("serialnumber").trim()%>'
           devicetypeid= '<%=request.getParameter("devicetypeid").trim()%>'
           parameterstring= '<%="devicetypeid=" + csd + "&" +
                "description=" + request.getParameter("description").trim() + "&" +
                "devicegroupid=" + request.getParameter("devicegroupid").trim() + "&" +
                "refconfigsetid=" + request.getParameter("refconfigsetid").trim() + "&" +
                "shortname=" + request.getParameter("shortname").trim() + "&" +
                "serialnumber=" + request.getParameter("serialnumber").trim() + "&" +
                "devicetypeid=" + request.getParameter("devicetypeid").trim() %>' />

        <pgs:logTXSuccess   userid= '<%= (String)session.getAttribute( "user" ) %>'
            orgname= '<%= (String)session.getAttribute( "orgname" ) %>'
            message= '<%= new String ("successully created device [" +
            request.getParameter( "shortname") + "]" )%>' />

        <pgs:generateCredentials/>


    </body>
</html>