<html>
<head>
<title>Admin Password Management</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<form method="post" action="">
  <table border="0" align="left" width="75%">
  <tr> 
	  <td class="formtitle">Manage Password 
        <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=($status == "1")? "Password changed successfully for Administrator": "&nbsp;"?></td>
  </tr>
  <tr> 
  	<td> 
	    <table border="0" class="bglist" align="left" cellpadding="4" cellspacing="1" width="100%">
          <tr> 
            <th colspan="2" height="31">Manage Password</th>
          </tr>
          <tr> 
            <td width="28%" height="29">Old Password</td>
            <td width="72%" height="29"> 
              <input type="password" name="textfield">
            </td>
          </tr>
          <tr> 
            <td width="28%">New Password</td>
            <td width="72%"> 
              <input type="password" name="newpin">
            </td>
          </tr>
          <tr> 
            <td width="28%">Confirm New Password</td>
            <td width="72%"> 
              <input type="password" name="newpinagain">
            </td>
          </tr>
          <tr> 
            <td colspan="2"> 
              <div align="center"> 
                <input type="hidden" name="status" value="1">
                <input type="submit" name="Submit" value="  Save  ">
                &nbsp;&nbsp; 
                <input type="reset" name="Submit2" value="Cancel">
              </div>
            </td>
          </tr>
        </table>
	</td>
	</tr>
	<tr>
		<td class="statustext">Note: To change the password of users, goto <a href="mailboxList.php" class="bgdark">Manage 
  		Mailboxes</a>, and click on <img src="../images/editicon.gif" width="12" height="12"> 
		corresponding to the user entry.</td>
	</tr>
</table>
</form>
  
</body>
</html>
