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
<script language="JavaScript">
    function checkDevicesAssigned() {
        if ( top.devicesAssigned.indexOf("cisco") != -1 ) {
            document.getElementById("hide").style.display = 'none';
        }
    }
</script>
</head>
<body onload="checkDevicesAssigned()">
<div id="topgroup" class="group">Phone Management</div>
<div id="hide" style="display:">
<div class="menuitem"><a href="speeddial.jsp?userid=<%=userid%>&stylesheet=details.xslt&mfgid=pingtel&devicetypeid=common&usertype=user" target="mainFrame">Manage Speed Dial</a></div>
<div class="menuitem"><a href="callhandling.jsp?userid=<%=userid%>&stylesheet=uucallhandling.xslt&mfgid=pingtel&devicetypeid=common&usertype=user" target="mainFrame">Manage Call Handling</a></div>
<div class="menuitem"><a href="applications.jsp?userid=<%=userid%>" target="mainFrame">Manage Applications</a></div>
</div>
<div class="menuitem"><a href="pinmgmnt.jsp?userid=<%=userid%>" target="mainFrame">Change PIN</a></div>
<div class="menuitem"><a href="user_about.jsp" target="mainFrame">About</a></div>
</body>
</html>
