<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:assignApplicationToUser userid='<%=request.getParameter("userid")%>'
	applicationid='<%=request.getParameter("applicationid")%>' />

<%
    response.sendRedirect( "/pds/ui/popup/form_assign_application.jsp?userid=" + request.getParameter( "userid" ) );;
%>
