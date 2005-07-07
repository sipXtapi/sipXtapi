<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
   response.sendRedirect( "list_all_applications.jsp" );
%>
<html>
<head>
<title>DeleteApplication</title></head>
<body>
<%-- Custom Tag to Delete an Application --%>
<pgs:deleteApplication applicationid= '<%=request.getParameter("applicationid")%>' />
</body>
</html>