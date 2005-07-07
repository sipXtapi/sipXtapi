<html>
<head>
<title>Settings</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
<script language="Javascript">
<!--
	function playGreeting()
	{
		var i = document.settingsform.stdGreeting.selectedIndex;
		var greetingLocation = document.settingsform.stdGreeting.options[i].value;
		setTimeout('location.href="' + greetingLocation + '"', 0);
	}
// -->
</script>
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<table border="0" align="left" width="75%">
  <tr> 
	  
    <td class="formtitle">Set System Defaults 
      <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=($status == "saved")? "<p class=\"statustext\">Settings Saved</p>" : "&nbsp;" ?></td>
  </tr>
  <tr> 
  	<td> 
		
      <table border="0" cellspacing="1" cellpadding="4" class="bglist" align="left" width="100%">
        <form method="post" name="settingsform" action="settings.php">
          <tr> 
            <th colspan=2 height="30">Media Server Settings</th>
          </tr>
          <tr> 
            <td colspan="2" height="35"><b>Security</b></td>
          </tr>
          <tr> 
            <td>Proxy Server:</td>
            <td> 
              <input type="text" name="proxyLocation" value="test.pingtel.com">
            </td>
          </tr>
          <tr> 
            <td>Registry Server:</td>
            <td> 
              <input type="text" name="registryLocation" value="test.pingtel.com">
            </td>
          </tr>
          <tr> 
            <td height="31">Realm:</td>
            <td height="31"> 
              <input type="text" name="defaultRealm" value="xpressa-appdev">
            </td>
          </tr>
          <tr> 
            <td height="31">Authentication Type:</td>
            <td height="31"> 
              <select name="authType">
                <option value="none">None</option>
                <option value="basic">Basic</option>
                <option value="digest" selected>Digest</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="31"><b>User Settings</b></td>
          </tr>
          <tr> 
            <td>Minimum User Password Length:</td>
            <td> 
              <input type="text" name="minPasswordLen" value="4" maxlength="2">
            </td>
          </tr>
          <tr> 
            <td>Maximum User Password Length:</td>
            <td> 
              <input type="text" name="maxPasswordLen" value="8" maxlength="2">
            </td>
          </tr>
          <tr> 
            <td>Default User Password:</td>
            <td> 
              <input type="text" name="defaultPassword" value="1234">
            </td>
          </tr>
          <tr> 
            <td height="35">Maximum number of messages: </td>
            <td height="35"> 
              <input type="text" name="maxMessages" value="100">
            </td>
          </tr>
          <tr> 
            <td>Default Folders (semi-colon separated list):</td>
            <td> 
              <input type="text" name="defaultFolders" value="inbox;saved;deleted;" size="40">
            </td>
          </tr>
          <tr> 
            <td>Root directory for user mailboxes:</td>
            <td> 
              <input type="text" name="rootDirectory" value="/usr/vmailstore" size="40">
            </td>
          </tr>
          <tr> 
            <td>Default message subject:</td>
            <td> 
              <input type="text" name="subject" value="Voice Message">
            </td>
          </tr>
          <tr> 
            <td>Default VXML script location:</td>
            <td> 
              <input type="text" name="vxmlScriptLocation" value="/usr/vxmlscripts/standard.vxml" size="40">
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="35"><b>Message Length</b></td>
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
              <input type="text" name="maxInMsgLength" value="300">
              secs </td>
          </tr>
          <tr> 
            <td>Outgoing messages: maximum length</td>
            <td> 
              <input type="text" name="maxOutMsgLength2" value="300">
              secs</td>
          </tr>
          <tr> 
            <td colspan="2" height="32"><b>Prompts</b></td>
          </tr>
          <tr> 
            <td>Minimum length</td>
            <td> 
              <input type="text" name="minGreetingsLen" value="2">
              secs </td>
          </tr>
          <tr> 
            <td>Maximum length</td>
            <td> 
              <input type="text" name="maxGreetingsLen" value="30">
              secs</td>
          </tr>
          <tr> 
            <td height="35" colspan="2"><b>Housekeeping</b></td>
          </tr>
          <tr> 
            <td height="31">Deleted folder: Auto delete messages</td>
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
            <td height="31">Inbox folder: Auto delete messages after</td>
            <td height="31"> 
              <select name="inboxFiles">
                <option value="7days" selected>7 days</option>
                <option value="1 month">1 month</option>
                <option value="nodelete">Dont Delete</option>
              </select>
            </td>
          </tr>
          <tr> 
            <td colspan="2" height="54"> 
              <div align="center"> 
                <input type="hidden" name="status" value="saved">
                <input type="submit" name="Save" value="Save">
                &nbsp;&nbsp;&nbsp; 
                <input type="reset" name="cancel" value="Cancel">
              </div>
            </td>
          </tr>
        </form>
      </table>
	</td>
</tr>
</table>
</body>
</html>
