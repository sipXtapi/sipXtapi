<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<script language="JavaScript">
<!--
function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments;
  document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2)
   eval(args[i]+".location='"+args[i+1]+"'");
}

//-->
</script>
<link rel="stylesheet" type="text/css" href="style/dms_sipxchange.css">
<title>About</title>
</head>
<body class="bglight">
<table width="600">
    <tr>
        <td width="50%" align="left" class="formtitle">About</td>
        <td width="50%" align="right">
            <a href="#" onclick="top.displayHelpPage('help/WebHelp/sipxchange.htm#about.htm');" class="greentext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<br/>
<h1 class="formtitle">Configuration Server</h1>
<table border="0" cellpadding="4" cellspacing="0" class="bglist" width="600">
    <tr>
        <th>Version</th>
    </tr>
    <tr>
        <td align="center"><pgs:getVersion/></td>
    </tr>
</table>
</body>
</html>
