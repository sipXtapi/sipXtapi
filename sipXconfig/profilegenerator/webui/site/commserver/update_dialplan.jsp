<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="error/error.jsp" %>
<%@ page language="Java" %>
<jsp:useBean id="updatebean" class="com.pingtel.commserver.beans.ProcessDialPlansBean" >
    <jsp:setProperty name="updatebean" property="*"/>
</jsp:useBean>
<%
    updatebean.updateXML();

    response.sendRedirect("dialplan_details.jsp");
%>