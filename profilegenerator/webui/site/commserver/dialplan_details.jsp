<%--
 -
 - Copyright (c) 2003 Pingtel Corp.  (work in progress)
 -
 - This is an unpublished work containing Pingtel Corporation's confidential
 - and proprietary information.  Disclosure, use or reproduction without
 - written authorization of Pingtel Corp. is prohibited.
 -
 - 
 - 
 - Author: 
 --%>
 
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<jsp:useBean id="getdp" class="com.pingtel.commserver.beans.GetDestinationsXMLBean">
    <jsp:setProperty name="getdp" property="stylesheet" value="dialplan.xslt" />
    <jsp:setProperty name="getdp" property="xmlfile" value="destinations.xml"  />
    <jsp:setProperty name="getdp" property="redirectURL" value="/commserver/dialplan_details.jsp"  />
</jsp:useBean>
<html>
<head>
<link rel="stylesheet" href="../style/dms.css" type="text/css">
<script src="script/jsFunctions.js"></script>
</head>
<body class="bglight" onLoad="MM_goToURL('top.frames[\'command\']','command/dialplan_cmd.html');" onunload="checkSaveFlag('dialplan')">

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
            <h1 class="list">Dial Plans</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/commserver/help/commserver/WebHelp/configsrvr.htm#dial_plans.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<form name="inputform" action="update_dialplan.jsp" method="post">
<jsp:getProperty name="getdp" property="htmlpage" />
</form>
</body>
</html>