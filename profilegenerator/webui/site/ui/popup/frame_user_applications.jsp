<%@ page errorPage="/ui/error/error.jsp" %>
<%
    String userID = request.getParameter( "userid" );
%>
<html>
<head>
<title></title>
</head>
<frameset rows="40,*" frameborder="NO" border="0" framespacing="0" cols="*">
    <frame name="topFrame"
        scrolling="no"
        src="app_select_frame.jsp?userid=<%=userID%>">
    <frame name="mainFrame"
        noresize
        scrolling="yes"
        src="form_assign_application.jsp?userid=<%=userID%>">
</frameset>
<noframes>
<body bgcolor="#FFFFFF" text="#000000">
</body>
</noframes>
</html>