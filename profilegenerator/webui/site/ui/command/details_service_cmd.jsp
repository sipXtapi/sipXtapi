<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
<head>
<title>details_service_cmd</title>
<script src="../script/jsCommand.js"></script>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">

</head>
<body class="bglight">

 <pgs:services  stylesheet="details_service_cmd.xslt" 
                   outputxml="false" 
                   process= '<%=request.getParameter("process")%>'
                   location= '<%=request.getParameter("location")%>' 
                   command= '<%=request.getParameter("command")%>'
                   redirect='<%=request.getParameter("redirect")%>' />   


</body>


</html>
