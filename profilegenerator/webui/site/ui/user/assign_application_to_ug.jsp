<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:assignApplicationToUserGroup usergroupid='<%=request.getParameter("usergroupid")%>'
	applicationid='<%=request.getParameter("applicationid")%>' />
<%
    response.sendRedirect( "/pds/ui/popup/form_assign_application_toug.jsp?usergroupid=" + request.getParameter( "usergroupid" ) );;
%>
