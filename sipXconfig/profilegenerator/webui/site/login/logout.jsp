<%
    session.invalidate();
    String lic = request.getParameter("lic");
%>
<html>
<head>
<script language="javascript">
    var lic = '<%=lic%>';

    function chkLic()
    {
        var layer2 = document.getElementById("Layer2");
        var layer3 = document.getElementById("Layer3");

        if ( lic == "false" ) {
            layer2.style.display = "none";
            layer3.style.display = '';
        }
        else {
            layer2.style.display = '';
            layer3.style.display = "none";
        }
    }
</script>
<title>Configuration Server</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="../style/dms.css" type="text/css">
</head>

<body class="bgdark" onload="chkLic()">
<div id="Layer1" style="position:absolute; left:0px; top:0px; width:1306px; height:78px; z-index:1"><img src="../image/logo.gif" width="305" height="71"><img src="../image/pingtelbar.gif" width="1000" height="71" border="0">
</div>
<div id="Layer2" style="position:absolute; left:0px; top:110px; width:673px; height:106px; z-index:2" class="statustitle">You have
successfully logged off of the Configuration Server, or your session has timed out. Would you like to
  <a href="../ui">log in again?</a></div>
<div id="Layer3" style="position:absolute; left:0px; top:110px; width:673px; height:106px; z-index:2" class="statustitle">The license
file for this server is invalid.  Please contact Pingtel Support.
</div>
</body>
</html>