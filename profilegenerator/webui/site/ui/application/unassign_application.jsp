<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "/pds/ui/popup/form_assign_application_toset.jsp?applicationsetid=" +
                request.getParameter( "applicationsetid" ) );;
%>
<pgs:unassignApplicationFromApplicatonGroup applicationsetid='<%=request.getParameter("applicationsetid")%>'
	applicationid='<%=request.getParameter("applicationid")%>' />
