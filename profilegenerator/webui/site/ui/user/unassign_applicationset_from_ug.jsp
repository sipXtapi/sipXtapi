<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:unassignApplicationSetFromUserGroup usergroupid='<%=request.getParameter("usergroupid")%>'
	applicationsetid='<%=request.getParameter("applicationsetid")%>' />
<%
    response.sendRedirect( "/pds/ui/popup/form_assign_applicationset_toug.jsp?usergroupid=" + request.getParameter( "usergroupid" ) );;
%>