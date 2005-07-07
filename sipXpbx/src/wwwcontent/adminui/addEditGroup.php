<html>
<head>
<title>Add New Group</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<?php if( $action == "edit") { ?>
	<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Manage Groups<hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?= ($status == "deleted") ? "Group deleted successfully" : "&nbsp;" ?></td>
  </tr>
  <tr> 
  	<td> 
		
      <table width="100%" border="0" cellspacing="1" cellpadding="4" class="bglist" align="left">
        <form method="post" action="groupsList.php">
          <tr> 
            <th colspan=2 height="30">Edit Mailbox Group</th>
          </tr>
          <tr> 
            <td width="33%" height="32">Group Name</td>
            <td width="67%" height="32"> 
              <input type="text" name="frmLGroupName" size="30" value="<?= $name ?>">
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="35"><b>Message Settings</b></td>
          </tr>
          <tr> 
            <td>Incoming messages: minimum length</td>
            <td> 
              <input type="text" name="minInMsgLength" value="2">
              secs </td>
          </tr>
          <tr> 
            <td>Incoming messages: maximum length</td>
            <td> 
              <input type="text" name="maxOutMsgLength2" value="300">
              secs </td>
          </tr>
          <tr> 
            <td height="35">Outgoing messages: maximum length</td>
            <td height="35"> 
              <input type="text" name="maxMessages" value="50">
              Mb </td>
          </tr>
          <tr> 
            <td height="35" colspan="2"><b>HouseKeeping</b></td>
          </tr>
          <tr> 
            <td height="31">Delete messages in &quot;Deleted&quot; folder:</td>
            <td height="31"> 
              <select name="deletedFiles">
                <option value="logout" selected>On Logout</option>
                <option value="3days">After 3 days</option>
                <option value="7days">After a week</option>
                <option value="user">When user explicitly deletes</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td height="31">Delete messages in &quot;Inbox&quot; folder after:</td>
            <td height="31"> 
              <select name="inboxFiles">
                <option value="7days">7 days</option>
                <option value="1 month">1 month</option>
                <option value="nodelete" selected>Dont Delete</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td colspan="2"> 
              <div align="center"> 
                <input type="submit" name="Submit" value="Save">
                &nbsp;&nbsp;&nbsp; 
                <input type="reset" name="cancel" value="Cancel">
              </div>
            </td>
          </tr>
          <input type="hidden" name="status" value="1">
        </form>
      </table>
		</td>
	</tr>
</table>
<?php } else { ?>
<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Manage Groups<hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?= ($status == "deleted") ? "Group deleted successfully" : "&nbsp;" ?></td>
  </tr>
  <tr> 
  	<td> 
		
      <table width="100%" border="0" cellspacing="1" cellpadding="4" class="bglist" align="left">
        <form method="post" action="groupsList.php">
          <tr> 
            <th colspan=2 height="30">Add New Mailbox Group</th>
          </tr>
          <tr> 
            <td width="33%" height="32">Group Name</td>
            <td width="67%" height="32"> 
              <input type="text" name="frmLGroupName" size="30">
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="35"><b>Message Settings</b></td>
          </tr>
          <tr> 
            <td>Incoming messages: minimum length</td>
            <td> 
              <input type="text" name="minInMsgLength" value="2">
              secs </td>
          </tr>
          <tr> 
            <td>Incoming messages: maximum length</td>
            <td> 
              <input type="text" name="maxOutMsgLength2" value="300">
              secs </td>
          </tr>
          <tr> 
            <td height="35">Outgoing messages: maximum length</td>
            <td height="35"> 
              <input type="text" name="maxMessages" value="50">
              Mb </td>
          </tr>
          <tr> 
            <td height="35" colspan="2"><b>HouseKeeping</b></td>
          </tr>
          <tr> 
            <td height="31">Delete messages in &quot;Deleted&quot; folder:</td>
            <td height="31"> 
              <select name="deletedFiles">
                <option value="logout" selected>On Logout</option>
                <option value="3days">After 3 days</option>
                <option value="7days">After a week</option>
                <option value="user">When user explicitly deletes</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td height="31">Delete messages in &quot;Inbox&quot; folder after:</td>
            <td height="31"> 
              <select name="inboxFiles">
                <option value="7days">7 days</option>
                <option value="1 month">1 month</option>
                <option value="nodelete" selected>Dont Delete</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td colspan="2"> 
              <div align="center"> 
                <input type="submit" name="Submit" value="Add Group">
                &nbsp;&nbsp;&nbsp; 
                <input type="reset" name="cancel" value="Cancel">
              </div>
            </td>
          </tr>
          <input type="hidden" name="status" value="1">
        </form>
      </table>
		</td>
	</tr>
</table>
<?php } ?>
</body>
</html>
