<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "list_all_applicationsets.jsp" );
%>
<html>
    <head>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    <title>EditApplicationSet</title>
    </head>
    <body>
        <pgs:editApplicationSet  name='<%=request.getParameter("name")%>'
           applicationsetid='<%=request.getParameter("applicationsetid")%>' />
    </body>
</html>
