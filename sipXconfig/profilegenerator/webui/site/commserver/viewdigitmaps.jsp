<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<jsp:useBean id="getdp" class="com.pingtel.commserver.beans.GetDestinationsXMLBean">
    <jsp:setProperty name="getdp" property="stylesheet" value="viewdigitmaps.xslt" />
    <jsp:setProperty name="getdp" property="xmlfile" value="destinations.xml"  />
</jsp:useBean>
<html>
<head>
<link rel="stylesheet" href="../style/dms.css" type="text/css">
<style type="text/css">
 .formtext {
    padding: 0px 10px;
}
</style>
<script language="javascript">
    function closeWindow() {
        this.window.close();
    }
</script>
</head>
<body class="bglight">
<h1 class="list">View Digitmaps</h1>
<br>
<p class="formtext">Based on your currently defined system-wide dial plans, the suggested digit maps for your devices are:</p>
<jsp:getProperty name="getdp" property="htmlpage" />
<p class="formtext">The dial plan portion of a digit map establishes the expected length and format of a typical, valid
telephone number. When an end user dials a number that has the same length and format as a predefined dial plan, the device
routes the call to the SIP address defined for that digit map automatically.</p>
<p class="formtext">You define digit maps at the device or device group level by selecting the Calling Out category.</p>

<div align="center"><input type="button" value="Close" onclick="closeWindow()"/></div>

</body>
</html>
