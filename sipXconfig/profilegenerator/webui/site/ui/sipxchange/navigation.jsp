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
<div class="menuitem"><a href="/sipxconfig/app?service=external/UserCallForwarding&sp=S<%=userid%>" 
    target="mainFrame">Manage Call Forwarding</a></div>
<div class="menuitem"><a href="pinmgmnt.jsp?userid=<%=userid%>" target="mainFrame">Change PIN</a></div>
<div class="menuitem"><a href="user_about.jsp" target="mainFrame">About</a></div>
<%
    if( session.getAttribute("mediaserverhost") != null )
    {
        String mediaserverHostName = (String) session.getAttribute("mediaserverhost");
%>
<div class="menuitem"><a href="<%= mediaserverHostName %>/userui/" target="voicemail">Voicemail</a></div>
<%
    }
%>
</body>
</html>
