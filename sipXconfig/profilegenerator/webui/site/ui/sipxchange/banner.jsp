<html>
<head>
<SCRIPT language="JavaScript">
<!--
function myDate()
{
	var today = new Date();
	var returnStr = today.toLocaleString();
	// Strip out time before returning the string
	return returnStr.substring(0, returnStr.indexOf(':') - 2);
}
//-->
</SCRIPT>
<SCRIPT src="jsFunctions.js"></SCRIPT>
<title>Banner</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
</head>

<body class="bgdark" topmargin="0" marginheight="0" leftmargin="0" marginwidth="0" onLoad="MM_preloadImages('images/logouton.gif','images/homeon.gif')">
<table width="100%" border="0" cellspacing=0 cellpadding=0 align="left" background="images/pingtelbar.gif">
  <tr>
    <td width="305" rowspan="2" valign="top"><img src="images/logo.gif" width="305" height="71" border=0></td>
    <td height="36" valign="top" align="right" colspan="2"><a href="index.jsp" target="_parent"><img src="images/homeoff.gif" width="41" height="20" border=0 name="Image1" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('Image1','','images/homeon.gif',1)"></a>
     <img src="images/spacer.gif" width="25"> <a href="../../login/logout.jsp" target="_top"><img src="images/logoutoff.gif" width="53" height="20" border=0 name="logout" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('logout','','images/logouton.gif',1)"></a>
      <img src="images/spacer.gif" width="10"></td>
  </tr>
  <tr>
    <td height="35" width="100%" colspan="2"><img src="images/headerbottom.gif" width="100%" height="35"></td>
  </tr>
  <tr>
    <td height="39" colspan="2" class="statustext" nowrap>
      <img src="images/spacer.gif" width="230" height="8">&nbsp;<span class="statustitle">User:</span>&nbsp;<%= session.getAttribute( "user" ).toString() %>
    </td>
    <td height="39" class="date" align="right" NOWRAP width="100%">
      <script type="text/javascript">document.write(myDate())</script>
      <img src="images/spacer.gif" width="10"></td>
  </tr>
</table>
</body>
</html>
