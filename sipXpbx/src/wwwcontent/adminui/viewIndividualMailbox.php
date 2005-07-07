<html>
<head>
<title>View Mailbox Information</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<?php 
$message = "&nbsp;";
if($status == "1")
	$message = "Mailbox added successfully for ". $frmLineId;
else if($status == "2")
	$message = "Credentials added successfully for ". $frmLineId;
?>
<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Manage inboxes 
      <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=$message ?></td>
  </tr>
	<tr>
		<td>
			
      <table border="0" cellspacing="1" cellpadding="4" class="bglist" align="left" width="100%">
        <tr> 
          <th colspan=2 height="30">inbox Info - 
            <?=$frmLineId ?>
          </th>
        </tr>
        <tr> 
          <td width="24%">inbox Group</td>
          <td width="76%">&nbsp;
            <?=$frmGroup ?>
          </td>
        </tr>
        <tr> 
          <td colspan="2"><b>SIP Credential<img src="/images/spacer.gif" width="20" height="8"> 
            <a href="addCredentials.php?action=edit&frmLineId=<?=$frmLineId ?>"><img src="/images/editicon.gif" width="12" height="12" border=0></a><img src="/images/spacer.gif" width="20" height="8"><img src="/images/del.gif" width="12" height="12" alt="Delete Credential"></b></td>
        </tr>
        <tr> 
          <td width="28%">Realm</td>
          <td width="72%">&nbsp;
            <?= ($frmRealm != "")? $frmRealm : "xpressa-appdev" ?>
          </td>
        </tr>
        <tr> 
          <td width="28%">User ID</td>
          <td width="72%">&nbsp;
            <?php if( strpos($frmLineId, "sip:") === false ){
				  		echo substr($frmLineId, 0, strpos($frmLineId, "@")) ;
					}
					else
					{
						echo substr($frmLineId, 4, strpos($frmLineId, "@") - 4) ;
					}
					?>
          </td>
        </tr>
        <tr> 
          <td width="28%">Authentication Type</td>
          <td width="72%">&nbsp;
            <?= ($frmAuthType != "")? $frmAuthType : "Digest" ?>
        </tr>
      </table>
		</td>
	</tr>
</table>
</body>
</html>
