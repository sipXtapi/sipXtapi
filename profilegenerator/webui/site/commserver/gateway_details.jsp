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
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<jsp:useBean id="getgw" class="com.pingtel.commserver.beans.GetDestinationsXMLBean">
    <jsp:setProperty name="getgw" property="stylesheet" value="gateway.xslt" />
    <jsp:setProperty name="getgw" property="xmlfile" value="destinations.xml"  />
    <jsp:setProperty name="getgw" property="redirectURL" value="gateway_details.jsp"  />
</jsp:useBean>
<html>
<head>
<link rel="stylesheet" href="../style/dms.css" type="text/css">
<script src="script/jsFunctions.js"></script>
</head>
<body class="bglight" onLoad="MM_goToURL('top.frames[\'command\']','command/gateways_cmd.html');" onunload="checkSaveFlag('gateway')">

<p id="error">
    <%
        String errorMessage = request.getParameter( "error_message" );

        if ( errorMessage != null ) {
            out.println( errorMessage );
        }
    %>
</p>
<table width="600">
    <tr>
        <td width="50%" align="left">
            <h1 class="list">Gateways</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/commserver/help/commserver/WebHelp/configsrvr.htm#gateways.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<form name="inputform" action="update_gateway.jsp" method="post">
<jsp:getProperty name="getgw" property="htmlpage" />
</form>
</body>
</html>