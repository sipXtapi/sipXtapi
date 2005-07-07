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
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>About</title>
</head>
<body class="bglight"
    onLoad="MM_goToURL('parent.frames[\'command\']','../command/command_blank.html')">
<table width="600">
    <tr>
        <td width="50%" align="left">
            <h1 class="list">About</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#about_configsrvr.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>
<br/>
<h1 class="about_heading">Configuration Server</h1>
<table border="0" cellpadding="4" cellspacing="0" class="bglist" width="600">
    <tr>
        <th>Version</th>
    </tr>
    <tr>
        <td align="center"><pgs:getVersion/></td>
    </tr>
</table>

<br/>

<br/>
<h1 class="about_heading">Database</h1>
<pgs:databaseVersion stylesheet="adminabout.xslt" outputxml="false" />
        <form action="../patch/upgrade_database.jsp" method="get">
            <table width="600" border="0">
                <tr>
                    <td align="center">
                        <input type="submit" value="Apply latest patches"/>
                    </td>
                </tr>
            </table>
            <input type="hidden" name="version" value="latest"/>
        </form>
</body>
</html>
