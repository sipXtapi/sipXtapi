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
<jsp:useBean id="updatebean" class="com.pingtel.commserver.beans.ProcessDialPlansBean" >
    <jsp:setProperty name="updatebean" property="*"/>
</jsp:useBean>
<%
    updatebean.updateXML();

    response.sendRedirect("dialplan_details.jsp");
%>