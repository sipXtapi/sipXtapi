<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:unassignApplicationSetFromUser userid='<%=request.getParameter("userid")%>'
	applicationsetid='<%=request.getParameter("applicationsetid")%>' />
<%
    response.sendRedirect( "/pds/ui/popup/form_assign_applicationset.jsp?userid=" + request.getParameter( "userid" ) );;
%>