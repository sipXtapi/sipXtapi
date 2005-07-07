<html>
<head>
<title>Add New Mailbox</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Manage inboxes 
      <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=($status == "deleted")? "Mailbox deleted successfully": "&nbsp;"?></td>
  </tr>
  <tr> 
  	<td> 
      <table width="100%" border="0" cellspacing="1" cellpadding="4" class="bglist" align="left">
        <form method="post" action="viewIndividualMailbox.php">
          <tr> 
            <th colspan=2 height="30"> 
              <?= ($action == "edit")? "Edit Credentials - $frmLineId":"Add Credentials - $frmLineId" ?>
            </th>
          </tr>
          <tr> 
            <td width="33%">Authentication Type</td>
            <td width="67%">&nbsp;Digest</td>
          </tr>
          <tr> 
            <td width="33%">Realm</td>
            <td width="67%">&nbsp;xpressa-appdev 
              <!--Get this value from global settings -->
            </td>
          </tr>
          <tr> 
            <td width="33%">User ID</td>
            <td width="67%">&nbsp;
              <?php if( strpos($frmLineId, "sip:") === false ){
				  		echo substr($frmLineId, 0, strpos($frmLineId, "@")) ;
					}
					else
					{
						echo substr($frmLineId, 4, strpos($frmLineId, "@") - 4) ;
					}
					?>
              <!--If Mailbox is "hari@pingtel.com", then default this field to "hari" -->
            </td>
          </tr>
          <tr> 
            <td width="33%">Password</td>
            <td width="67%"> 
              <input type="password" name="frmPassword">
            </td>
          </tr>
          <tr> 
            <td width="33%">Confirm Password</td>
            <td width="67%"> 
              <input type="password" name="frmPasswordConfirm">
              <!--Get this from the global settings -->
            </td>
          </tr>
          <tr> 
            <td colspan="2"> 
              <div align="center"> 
                <input type="submit" name="Save" value="  Save  ">
                <img src="/images/spacer.gif" width="40" height="1"> 
                <input type="reset" name="cancel" value="Cancel">
              </div>
            </td>
          </tr>
          <input type="hidden" name="status" value="2">
          <input type="hidden" name="frmLineId" value="<?= $frmLineId ?>">
          <input type="hidden" name="frmGroup" value="<?= $frmGroup ?>">
          <input type="hidden" name="frmSize" value="<?= $frmSize ?>">
        </form>
      </table>
		</td>
	</tr>
</table>
</body>
</html>
