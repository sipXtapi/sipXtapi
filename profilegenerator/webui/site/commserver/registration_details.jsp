<%@ page import="java.util.ResourceBundle"%> <%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
 
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<jsp:useBean id="getReg" class="com.pingtel.commserver.beans.GetRegistrationsXMLBean">
    <jsp:setProperty name="getReg" property="stylesheet" value="registrations.xslt" />
    <jsp:setProperty name="getReg" property="xmlfile" value="registration.xml"  />
    <jsp:setProperty name="getReg" property="redirectURL" value="error.jsp"  />
    <jsp:setProperty name="getReg" property="sort" value='<%=request.getParameter("sort")%>'/>
    <jsp:setProperty name="getReg" property="index" value='<%=request.getParameter("index")%>'/>
    <jsp:setProperty name="getReg" property="count" value='<%=request.getParameter("count")%>'/>
    <jsp:setProperty name="getReg" property="max" value='<%=request.getParameter("max")%>'/>
    <jsp:setProperty name="getReg" property="location" value='<%=request.getParameter("location")%>'/>
    <jsp:setProperty name="getReg"  property="resourcebundle" value='<%=ResourceBundle.getBundle("webui-resources")%>'/>
</jsp:useBean>
<html>
<head>
<link rel="stylesheet" href="../style/dms.css" type="text/css">
<style type="text/css">
table.bglist a:link{
	font-family : Arial, Helvetica, sans-serif;
	font-size: 14px;
	font-weight : bolder;
	color : #FFFACD;
	background-color : #669999;
}

table.bglist a:visited{
	font-family : Arial, Helvetica, sans-serif;
	font-size: 14px;
	font-weight : bolder;
	color : #FFFACD;
	background-color : #669999;
}
table.bglist a:hover{
	font-family : Arial, Helvetica, sans-serif;
	font-size: 14px;
	font-weight : bolder;
	color : #FFCF31;
	background-color : #669999;
}
a.navItem{
	font-family : Arial, Helvetica, sans-serif;
	font-size: 12px;
	font-weight : bold;
	color : #0031CE;
	background-color : #FFFFCC;
	padding: 4px;
	text-decoration: none;
}

div.nav{
	display: block;
	padding: 4px;
}
</style>
<script src="script/jsFunctions.js"></script>
</head>
<body class="bglight"
    onLoad="MM_goToURL('top.frames[\'command\']','command/registrations_cmd.jsp?location=<%=request.getParameter("location")%>');">

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
            <h1 class="list">Registrations</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/commserver/help/commserver/WebHelp/configsrvr.htm#dial_plans.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<jsp:getProperty name="getReg" property="htmlpage" />

</body>
</html>
