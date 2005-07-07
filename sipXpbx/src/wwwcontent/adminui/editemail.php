<html>
<head>
<title>Email Notification</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
</head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">
<table border="0" align="left" width="75%">
  <tr> 
	<td class="formtitle">Email Notification
<hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?=($status == "saved")? "Call handling settings saved": "&nbsp;"?></td>
  </tr>
  <tr> 
  	<td class="statustext">To forward voicemail notifications to an EMail server, please specify the information 
  below. If you do not have this information, please contact your Internet Service 
  Provider (ISP). </td>
  </tr>
  <tr>
  	<td>
		
      <table width="100%" border="0" class="bglist" align="left" cellpadding="4" cellspacing="1">
        <tr> 
			
          <th colspan="2" height="19">Mail Account Information</th>
		  </tr>
		  <tr> 
			
          <td width="238">Mail Server</td>
			
          <td width="309"> 
            <input type="text" name="textfield4">
			</td>
		  </tr>
		  <tr> 
			
          <td width="238">Mail Server Username</td>
			
          <td width="309"> 
            <input type="text" name="textfield3">
			</td>
		  </tr>
		  <tr> 
			
          <td width="238">Mail Server Password</td>
			
          <td width="309"> 
            <input type="text" name="textfield2">
			</td>
		  </tr>
		  <tr>
			
          <td width="238">Mail Server Port</td>
			
          <td width="309"> 
            <input type="text" name="textfield" value="110">
			</td>
		  </tr>
		  <tr> 
			
          <td width="238"> 
            <div align="right"> 
              <input type="submit" name="Submit2" value="Save">
            </div>
			</td>
			
          <td width="309"> 
            <input type="reset" name="Reset" value="Cancel">
          </td>
		  </tr>
		</table>
	</td>
	</tr>
</table>
</body>
</html>
