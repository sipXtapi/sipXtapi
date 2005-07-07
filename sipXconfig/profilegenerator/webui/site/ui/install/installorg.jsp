<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="/ui/error/error.jsp" %>

<jsp:useBean id="installOrg" class="com.pingtel.pds.pgs.beans.InstallOrganizationBean" />
<jsp:setProperty name="installOrg" property="*"/>

<%
    // Redirect to login page
    response.sendRedirect( "../index.jsp" );
	installOrg.install();
    //Kill the session
    session.invalidate();
%>

