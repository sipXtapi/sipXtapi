<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    response.sendRedirect( "list_all_applications.jsp" );
%>
<html>
    <head>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    <title>EditApplication</title>
    </head>
    <body>
        <pgs:editApplication  name= '<%=request.getParameter("name")%>'
           applicationid= '<%=request.getParameter("applicationid")%>'
           url= '<%=request.getParameter("url")%>'
           description= '<%=request.getParameter("description")%>' />
    </body>
</html>
