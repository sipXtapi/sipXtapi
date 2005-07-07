<html>
<head>
<title>E-Mail Notification</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
<script language="JavaScript">
<!--
function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments; document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2) eval(args[i]+".location='"+args[i+1]+"'");
}
//-->
</script>
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<table border="0" align="left" width="75%">
  <tr> 
    <td class="formtitle">Email Notification </td>
    <td class="formtitle" align="right"><img src="../images/question4.gif" width="11" height="19"></td>
  </tr>
  <tr> 
    <td class="formtitle" colspan="2"> 
      <hr class="dms">
    </td>
  </tr>
  <tr> 
    <td class="errortext_light" colspan="2"> 
      <?=($status == "saved")? "Call handling settings saved": "&nbsp;"?>
    </td>
  </tr>
  <tr> 
    <td colspan="2"> 
      <table border="0" class="bglist" cellpadding="4" cellspacing="1" align="center" width="100%">
        <tr> 
          <th width="120">Server Name</th>
          <th width="100">User Name</th>
          <th width="54">Edit</th>
          <th width="105">Delete</th>
        </tr>
        <tr> 
          <td width="120">pop.acme.com</td>
          <td width="100">foo</td>
          <td width="54"> 
            <div align="center"><img src="/images/editicon.gif" width="12" height="12"></div>
          </td>
          <td width="105"> 
            <div align="center"><img src="/images/del.gif" width="12" height="12"></div>
          </td>
        </tr>
        <tr> 
          <td width="120">pop.abc.com</td>
          <td width="100">mozilla</td>
          <td width="54"> 
            <div align="center"><img src="/images/editicon.gif" width="12" height="12"></div>
          </td>
          <td width="105"> 
            <div align="center"><img src="/images/del.gif" width="12" height="12"></div>
          </td>
        </tr>
      </table>
    </td>
  </tr>
  <tr> 
    <td colspan="2"><a class="bgdark" href="editemail.php">Add Mail Server</a></td>
  </tr>
  <tr> 
    <td class="statustext" colspan="2"> 
      <p><br>
        You can configure your account to forward voice mail notifications. Click 
        the&quot;Add Mail Server&quot; button to configure additional accounts. 
        You can specify up to 3 servers.<br>
        <br>
        Bonus Points: Add Format Customization</p>
    </td>
  </tr>
</table>
</body>
</html>
