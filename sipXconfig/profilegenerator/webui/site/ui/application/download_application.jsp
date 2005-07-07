<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs"%>
<%
    String last = null;
    if( request.getParameter("cat") != null )
    {
        last = request.getParameter("cat");
    }
%>
<html>
    <head>
    <script language="JavaScript">
        function reloadForm()
        {
            var last = '<%=last%>';
            top.location = "/pds/ui/popup/downloadapps_frameset.html?last=" + last;
        }
    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    <title>CreateApplication</title>
    </head>
    <body class="bglight" onload="reloadForm()">
    <pgs:downloadApplications
        sourceurl='<%=request.getParameter("sourceurl")%>'
        applicationname='<%=request.getParameter("applicationname")%>'
        applicationdescription='<%=request.getParameter("applicationdescription")%>'
        devicetypeid='<%=request.getParameter("devicetypeid")%>'
        refpropertyid='<%=request.getParameter("refpropertyid")%>'
        shouldcreate='<%=request.getParameter("shouldcreate")%>'
        />
</body>
</html>