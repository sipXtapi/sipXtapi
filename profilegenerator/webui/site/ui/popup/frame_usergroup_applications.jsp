<%@ page errorPage="/ui/error/error.jsp" %>
<%
    String userGroupID = request.getParameter( "usergroupid" );
%>
<html>
<head>
<title></title>
</head>
<frameset rows="40,*" frameborder="NO" border="0" framespacing="0" cols="*">
    <frame name="topFrame"
        scrolling="no"
        src="app_ug_select_frame.jsp?usergroupid=<%=userGroupID%>">
    <frame name="mainFrame"
        noresize
        scrolling="yes"
        src="form_assign_application_toug.jsp?usergroupid=<%=userGroupID%>">
</frameset>
<noframes>
<body bgcolor="#FFFFFF" text="#000000">
</body>
</noframes>
</html>