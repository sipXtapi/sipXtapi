<%
   // get info from the session
   String user = session.getAttribute( "user" ).toString();
   String xsltDir = session.getAttribute( "xsltBaseDir" ).toString();
   String orgid = session.getAttribute( "orgid" ).toString();
   String stereotype = session.getAttribute( "stereotype" ).toString();
%>
<html>
<head>
<title>Welcome to DMS</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="../style/dms.css" type="text/css">
</head>

<body class="bglight">
<h3>Changes as of 2/26/2002</h3>
<hr>
<br>
<ol>
  <li>Button images have been changed to a slimmer look</li>
  <li>Application Set details shows tabs</li>
  <li>All create functionality, with the exception of admins works</li>
  <li>Dynamic population of a &lt;select&gt; box can be seen on &quot;add application&quot;</li>
  <li>The copy and delete functionality is now working in IE5, however there are 
    still some problems with the page refresh</li>
</ol>
<hr>
<h3>Changes as of 3/6/2002</h3>
<hr>
<br>
<ol>
  <li>Tabs for device details is working.  Click on List All Devices, then click on a device. 
  		There is a problem with the loading of the page, so once it's finished loading, click on the 
		first (General) tab to clear the page (you'll see what I mean)
	</li>

</ol>
</body>
</html>
