<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:unassignDevice deviceid='<%=request.getParameter("deviceid")%>'/>

<%
    response.sendRedirect( "/pds/ui/popup/form_assign_device.jsp?userid=" + request.getParameter( "userid" ) +
            "&displayid=" + request.getParameter("displayid") );
%>