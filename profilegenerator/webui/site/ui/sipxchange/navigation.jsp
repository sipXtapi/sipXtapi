<%
   // get userid from session
   String userid = session.getAttribute( "userid" ).toString();
%>
<html>
<head>
<title>navigation.gif</title>
<meta http-equiv="Content-Type" content="text/html">
<style type="text/css">
body {background: #639A9C; color: white;}
div#topgroup {margin-top: 2px;}
a {text-decoration: none;}
a:link {color: white;}
a:visited {color: white;}
a:hover {color: #FFCF31;}
div.group {font: 12pt Arial, Helvetica, Verdana, serif;
	font-weight: bolder;
	padding: 4px 0;
	margin-left: 8px;}
div.menuitem {font: 9pt Arial, Helvetica, Verdana, serif;
	margin-left: 8px;
	padding: 6px 0 8px 10px; }

</style>
</head>
<body>
<a class="menuitem" href="/sipxconfig/app?service=external/UserCallForwarding&sp=S<%=userid%>" 
    target="mainFrame">Manage Call Forwarding</a>
<a class="menuitem" href="pinmgmnt.jsp?userid=<%=userid%>" target="mainFrame">Change PIN</a>
<a class="menuitem" href="user_about.jsp" target="mainFrame">About</a>
<%
    if( session.getAttribute("mediaserverhost") != null )
    {
        String mediaserverHostName = (String) session.getAttribute("mediaserverhost");
%>
<a class="menuitem" href="<%= mediaserverHostName %>/userui/" target="voicemail">Voicemail</a>
<%
    }
%>
</body>
</html>
