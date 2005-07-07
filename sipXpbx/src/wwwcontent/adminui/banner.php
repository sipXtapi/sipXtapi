<html>
<head>
<script language="JavaScript">
<!--
function myDate()
{
	var today = new Date();
	var returnStr = today.toLocaleString();
	// Strip out time before returning the string
	return returnStr.substring(0, returnStr.indexOf(':') - 3);
}

function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}
//-->
</script>
<title>Banner</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body class="bgdark" topmargin="0" marginheight="0" leftmargin="0" marginwidth="0" onLoad="MM_preloadImages('/images/homeon.gif')">
<table width="100%" border="0" cellspacing=0 cellpadding=0 align="left" background="/images/pingtelbar.gif">
  <tr> 
    <td width="305" rowspan="2" valign="top"><a href="frameset.php" target="_parent"><img src="/images/logo.gif" width="305" height="71" border=0></a></td>
    <td width="100%" height="36" valign="top" align="right"> 
      <a href="frameset.php" target="_parent"><img src="/images/homeoff.gif" width="41" height="20" border=0 name="Image1" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('Image1','','/images/homeon.gif',1)"></a> 
      <img src="/images/spacer.gif" width="25"> <a href="index.php" target="_parent"><img src="/images/logoutoff.gif" width="53" height="20" border=0 name="logout" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('logout','','/images/logouton.gif',1)"></a> 
      <img src="/images/spacer.gif" width="10"></td>
  </tr>
  <tr>
  	<td width="100%" height="35" align="right"><img src="/images/headerbottom.gif" width="100%" height="35"></td>
  </tr>
  <tr> 
    <td height="39" width="305"> 
      <div align="left"><h1 class="date"><img src="/images/spacer.gif" width="10">Voice Mail Server - Administration Console</h1></div>
    </td>
    <td height="39" class="date" width="100%" align="right">
		<script type="text/javascript">document.write(myDate())</script>
		<img src="/images/spacer.gif" width="10">
	</td>
  </tr>
</table>
</body>
</html>
