<html>
<head>
<title>Add New Mailbox</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<?php if( $status == "edit") { ?>
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
			
      <table border="0" cellspacing="1" cellpadding="4" class="bglist" width="100%">
        <form method="post" action="viewIndividualMailbox.php" enctype="multipart/form-data">
          <tr> 
            <th colspan=2>Edit inbox - 
              <?= $frmLineId ?>
            </th>
          </tr>
          <tr> 
            <td colspan="2"><b>inbox Information</b></td>
          </tr>
		  <tr> 
            <td width="30%" align="right">inbox<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="text" name="frmLineId" value="<?= $frmLineId ?>" size="30">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">inbox Group<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <select name="frmGroup">
                <option value="staff" <?= ($frmGroup == "staff")? "selected":"" ?>>Staff</option>
                <option value="stafflarge">Staff WLarge Mailbox</option>
                <option value="executives" <?= ($frmGroup == "executives")? "selected":"" ?>>Executives</option>
                <option value="managers">Managers</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="frmPassword" value="1234">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Confirm Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="frmConfirmPassword" value="1234">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">VXML Script<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="radio" name="radiobutton" value="radiobutton" checked>
              Use Default<br>
              <input type="radio" name="radiobutton" value="radiobutton">
              Use my script 
              <input type="file" name="file">
            </td>
          </tr>
          <tr> 
            <td colspan="2"><b>SIP Credential Information</b></td>
          </tr>
          <tr> 
            <td width="30%" align="right">Authentication Type<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> Digest</td>
          </tr>
          <tr> 
            <td width="30%" align="right">Realm<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> xpressa-appdev</td>
          </tr>
          <tr> 
            <td width="30%" align="right">User ID<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> Resource id in Mailbox value entered above - automatically 
              retrieved </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="credential1Pwd" value="1234">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Confirm Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="credential1ConfirmPwd" value="1234">
            </td>
          </tr>
          <tr> 
            <td colspan="2"> 
              <div align="center"> 
                <!--May be "Add Credentials" comes here. -->
                <input type="submit" name="Submit" value="Save ">
                &nbsp;&nbsp;&nbsp; 
                <input type="reset" name="cancel" value="Cancel ">
              </div>
            </td>
          </tr>
          <input type="hidden" name="status" value="3">
        </form>
      </table>
		</td>
	</tr>
	<tr>
		<td class="statustext"><br>
    </td>
	</tr>
</table>
<?php } else { ?>
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
            <th colspan=2 height="30">Add New inbox</th>
          </tr>
          <tr> 
            <td colspan="2"><b>inbox Information</b></td>
          </tr>
          <tr> 
            <td width="30%" align="right">inbox<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="text" name="frmLineId" value="sip:" size="30">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">inbox Group<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <select name="frmGroup">
                <option value="staff" selected>Staff</option>
                <option value="stafflarge">Staff WLarge Mailbox</option>
                <option value="executives">Executives</option>
                <option value="managers">Managers</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">VXML Script<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="radio" name="radiobutton" value="radiobutton" checked>
              Use Default<br>
              <input type="radio" name="radiobutton" value="radiobutton">
              Use my script 
              <input type="file" name="file">
            </td>
          </tr>
          <tr> 
            <td colspan="2"><b>SIP Credential Information</b></td>
          </tr>
          <tr> 
            <td width="30%" align="right">Authentication Type<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> Digest 
              <!--Get this value from the global settings file -->
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Realm<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> xpressa-appdev 
              <!-- Read this value from global server settings -->
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">User ID<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> Resource id in Mailbox value entered above - retrieved 
              automatically 
              <!--If user enters "sip:hari@pingtel.com" in the mailbox field, populate this field automatically to the value of 'hari'. -->
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="credential1Pwd">
            </td>
          </tr>
          <tr> 
            <td width="30%" align="right">Confirm Password<img src="/images/spacer.gif" width="8" height="8"></td>
            <td width="70%"> 
              <input type="password" name="credential1ConfirmPwd">
            </td>
          </tr>
          <tr> 
            <td colspan="2" align="center"> 
              <input type="submit" name="Submit" value="Save">
              <img src="/images/spacer.gif" width="20" height="8"> 
              <input type="reset" name="cancel" value="Cancel ">
            </td>
          </tr>
          <input type="hidden" name="status" value="1">
        </form>
      </table>
		</td>
	</tr>
	<tr>
		
    <td class="statustext">&nbsp;</td>
	</tr>
</table>
<?php } ?>
</body>
</html>
