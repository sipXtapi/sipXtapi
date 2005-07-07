<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "/pds/ui/popup/form_assign_applicationset.jsp?userid=" + request.getParameter( "userid" ) );;
%>

<pgs:assignApplicationSetToUser userid='<%=request.getParameter("userid")%>'
	applicationsetid='<%=request.getParameter("applicationsetid")%>' />

