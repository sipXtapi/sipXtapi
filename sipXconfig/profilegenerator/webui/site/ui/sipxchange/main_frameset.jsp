<%@ page errorPage="/ui/error/error.jsp"%>
<%
   // get userid from session
   String userid = request.getParameter( "userid" );
%>

<html>
<head>
<title>Pingtel Configuration Server</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>
	<frameset rows="60,*" frameborder="NO" border="0" framespacing="0"> 
  		<frame name="heading" scrolling="NO" noresize src="mainFrameHeading.jsp?multiservers=no">
		<frame name="navFrame" scrolling="NO" noresize src="main.jsp?userid=<%= userid %>">
	</frameset>
<noframes> 
<body bgcolor="#FFFFFF" text="#000000">
</body>
</noframes> 
</html>
