<html>
<head>
<title>Index Main</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
<script language="JavaScript">
<!--
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
</head>

<body class="bgdark" topmargin="0" marginheight="0" leftmargin="0" marginwidth="0">
<table width="100%" border="0" cellspacing=0 cellpadding=0 align="left">
  <tr> 
    <td colspan="2" align="center" vailgn="middle"> 
      <p>&nbsp;</p>
      <p>&nbsp;</p>
      <form method="post" action="frameset.php" target="_parent">
        <table width="399" border="0" bgcolor="#FFFFCC" align="center" class="bglight" cellpadding="0" cellspacing="0">
          <tr> 
            <td colspan="2" class="formtitle" align="center" height="66">Voice Mail Server - Admin UI Login 
              <hr class="dms">
            </td>
          </tr>
          <tr> 
            <td width="39%" height="30" align="right" class="formtext">username</td>
            <td height="30" width="61%"> <img src="/images/spacer.gif" align="left" width="10" height="1"> 
              <input type="text" name="lineid">
            </td>
          </tr>
          <tr> 
            <td width="39%" height="30" align="right" class="formtext">password</td>
            <td height="30" width="61%"> <img src="/images/spacer.gif" align="left" width="10" height="1"> 
              <input type="password" name="password">
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="40" align="center"><input type="image" border="0" name="imageField" src="/images/go_off.gif" onMouseOver="MM_swapImage('imageField','','/images/go_on.gif',0);" onMouseOut="MM_swapImgRestore()"></td>
          </tr>
        </table>
      </form>
    </td>
  </tr>
</table>
</body>
</html>
