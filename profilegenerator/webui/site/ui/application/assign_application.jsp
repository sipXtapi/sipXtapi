<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:assignApplicationToApplicatonGroup applicationsetid='<%=request.getParameter("applicationsetid")%>'
	applicationid='<%=request.getParameter("applicationid")%>' />

<%
    response.sendRedirect( "/pds/ui/popup/form_assign_application_toset.jsp?applicationsetid=" +
                request.getParameter( "applicationsetid" ) );;
%>