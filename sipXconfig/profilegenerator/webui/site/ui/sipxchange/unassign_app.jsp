<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "applications.jsp?userid=" + request.getParameter( "userid" ) );
%>
<pgs:unassignApplicationFromUser userid='<%=request.getParameter("userid")%>'
	applicationid='<%=request.getParameter("applicationid")%>' />
