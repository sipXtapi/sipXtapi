<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<title>DeleteApplication</title></head>
<body>
<%-- Custom Tag to Delete an Application --%>
<pgs:deleteApplication applicationid= '<%=request.getParameter("applicationid")%>' />
<%
   response.sendRedirect( "list_all_applications.jsp" );
%>
</body>
</html>