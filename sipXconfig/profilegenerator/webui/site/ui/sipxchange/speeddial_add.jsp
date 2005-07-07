<html>
<head>
<title>Add Speeddial</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
<SCRIPT src="jsFunctions.js"></SCRIPT>
</head>

<body class="bglight">
<table width="85%" border="0" align="left">
  <tr>
		<td><img src="images/spacer.gif" width=5></td>
		<td>
			
        <table width="100%" border="0">
          <tr> 
            <td class="formtitle" height="30" width="95%">Manage Speed Dial</td>
            <td height="30" width="5%"><a class="greentext" href="javascript:void 0" onclick="displayHelpPage('help/speeddial.html');">Help</a></td>
          </tr>
          <tr> 
            <td class="errortext_light" colspan="2"> 
              <hr class="dms">
            </td>
          </tr>
          <tr> 
            <td colspan=2 class="errortext_light"> 
			<%
				if(request.getParameter("id") != null && request.getParameter("number") != null)
					out.println("Speed dial number " + request.getParameter("number") + " saved as " + request.getParameter("id"));
			%>				
			</td>
          </tr>
          <tr> 
            <td class="list" colspan="2">
			<form method="post" name="speed">
              <table class="bglist" cellspacing="1" cellpadding="6" border="0" width="100%">
                <tr> 
                  <th align="center" colspan="2">Add Speed dial</th>
                </tr>
                <tr> 
                  <td width="22%">Speed dial ID</td>
                  <td width="78%"> 
                    <input type="text" name="id">
                  </td>
                </tr>
                <tr> 
                  <td width="22%">Label</td>
                  <td width="78%"> 
                    <input type="text" name="label">
                  </td>
                </tr>
                <tr> 
                  <td width="22%">Speed dial Type</td>
                  <td width="78%"> 
                    <input type="radio" name="type" value="number" onclick='document.speed.number.value=""'>Phone number
					&nbsp;&nbsp;
                    <input type="radio" name="type" value="url" onclick='document.speed.number.value="sip:"'>SIP URL
				  </td>
                </tr>
                <tr> 
                  <td width="22%">Phone number / URL</td>
                  <td width="78%"> 
                    <input type="text" name="number">
                  </td>
                </tr>
                <tr align="left"> 
                  <td colspan="2"> <img src="images/spacer.gif" width="150" height="8"> 
                    <input type="submit" name="Submit" value="Save">
                    <img src="images/spacer.gif" width="1" height="1">
					<input type="button" name="Submit2" value="Cancel" onclick="mainframe.location.href=\"speeddial.htm\"">
                  </td>
                </tr>
              </table>
			  </form>
            </td>
          </tr>
        </table>
</td></tr></table>
<p>&nbsp;</p>
</body>
</html>
