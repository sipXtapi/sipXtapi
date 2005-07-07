<html>
<head>
<title>Mailbox List</title>
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

<body class="bglight">
<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Manage inboxes 
      <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=($status == "deleted")? "Mailbox deleted successfully": "&nbsp;"?></td>
  </tr>
  <!-- Commented out for the prototype demo. But will need to put it back
  <tr>
		
    <td> <img src="/images/spacer.gif" width="1" height="25"> <a class="bgdark" href="addEditMailbox.php?status=add"><img src="/images/buttons/pillshaped3.gif" name="Image1" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('Image1','','/images/buttons/pillshaped4.gif',0)" border="0" align="top"></a> 
    </td>
  </tr>
  -->
  <tr> 
  	<td> 
      <table class="bglist" cellspacing="1" cellpadding="6" border="0" width="100%">
        <tr> 
          <th width="37%">inbox</th>
          <th width="37%">Group</th>
          <th width="13%">Edit</th>
          <th width="13%">Delete</th>
        </tr>
        <tr> 
          <td width="37%">jcoffey@test.pingtel.com</td>
          <td width="37%">Staff</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=jcoffey@test.pingtel.com&frmGroup=staff&status=edit"><img src="../images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
        <tr> 
          <td width="37%">hari@test.pingtel.com</td>
          <td width="37%">Staff</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=hari@test.pingtel.com&frmGroup=staff&status=edit"><img src="/images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
        <tr> 
          <td width="37%">rschaaf@test.pingtel.com</td>
          <td width="37%">Executives</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=rschaaf@test.pingtel.com&frmGroup=executives&status=edit"><img src="../images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
        <tr> 
          <td width="37%">dpetrie@test.pingtel.com</td>
          <td width="37%">Executives</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=dpetrie@test.pingtel.com&frmGroup=executives&status=edit"><img src="../images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
        <tr> 
          <td width="37%">jross@test.pingtel.com</td>
          <td width="37%">Managers</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=jross@test.pingtel.com&frmGroup=managers&status=edit"><img src="../images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
        <tr> 
          <td width="37%">kdick@test.pingtel.com</td>
          <td width="37%">Staff with large mailboxes</td>
          <td align="center" width="13%"><a href="addEditMailbox.php?frmLineId=kdick@test.pingtel.com&frmGroup=staff&status=edit"><img src="../images/editicon.gif" width="12" height="12" border="0"></a></td>
          <td align="center" width="13%"><a href="mailboxList.php?status=deleted" onClick="return confirm('Do you want to proceed with the delete?');"><img src="/images/del.gif" border=0 width="12" height="12" alt="Delete Mailbox"></a></td>
        </tr>
      </table>
	  </td>
	</tr>
	<tr>
		<td>
		<img src="/images/spacer.gif" width="1" height="30">
      <a class="bgdark" href="addEditMailbox.php?status=add"><img src="/images/buttons/addmailbox_up.gif" name="Image2" onMouseOut="MM_swapImgRestore()" onMouseOver="MM_swapImage('Image2','','/images/buttons/addmailbox_down.gif',0)" border="0" align="bottom"></a></td>
	</tr>
</table>
</body>
</html>
