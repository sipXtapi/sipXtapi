<%--
 -
 - Copyright (c) 2003 Pingtel Corp.  (work in progress)
 -
 - This is an unpublished work containing Pingtel Corporation's confidential
 - and proprietary information.  Disclosure, use or reproduction without
 - written authorization of Pingtel Corp. is prohibited.
 -
 - $File: //depot/main/tools/IDEA/fileTemplates/includes/JSPFileHeader.java $
 - $Revision: #1 $
 - Author: 
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