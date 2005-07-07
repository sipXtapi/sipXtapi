<html>
<head>
<title>Distribution List</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/voicemail.css" type="text/css">
<script language="JavaScript">
<!--
function Si()
{
	var a= document.all, f= a.first.value, l= a.left, i= l.length, x= 0;
	while(i-- > 0)
	{
		if(f.toLowerCase() <= l[i].text.toLowerCase())
			x= i;
	}
	l.selectedIndex= x;
}

function Mv(f, t)
{
	var i= f.length, m= false, e= m;
	while(i-- > 0)
	{
		if(f[i].selected)
		{
			if(f[i]._m > 0)
				e= true;
			else
			{
				var newopt= document.createElement("option");
				newopt.value= f[i].value;
				newopt.text= f[i].text;
				f.remove(i);
				t.add(newopt);
				m= true;
			}
		}
	}
	if(e)
		alert("Some members cannot be removed because this distribution list\n"
			+ "corresponds to their user group.");
	
}

function Ad()
{
	var a= document.all;
	Mv(a.left, a.right);
}

function Rm()
{
	var a= document.all;
	Mv(a.right, a.left);
}
//-->
</script>
</head>

<body class="bglight">
<form name="form3" method="post" action="distributionList.php">
<table border="0" align="left" width="75%">
  <tr> 
	  <td class="formtitle">Distribution Lists 
        <hr class="dms"></td>
  </tr>
  <tr> 
	<td class="errortext_light"><?= ($status == "deleted") ? "Group deleted successfully" : "&nbsp;" ?></td>
  </tr>
  <tr> 
  	<td> 
		<table border="0" align="left" class="bglist" cellspacing="1" cellpadding="4" width="100%">
          <tr> 
			<th colspan=2>
			<?php if($status == "add") { 
				echo "Add New Distribution List";
			} else {
				echo "Edit Distribution List";
			}
			?>
			</th>
		  </tr>
		  <tr> 
			<td width="12%">Name:</td>
			<td width="88%"> 
              <?php if($status == "add") { ?>
              <input type="text" name="frmDistName">
			<?php } else { ?>
				<input type="text" name="frmDistName" value="<?php echo $name ?>">
			<?php } ?>
			</td>
		  </tr><tr><td colspan=2>
		      <table cellspacing=0 cellpadding=4 border=0 class="bglist" width="100%">
                <tr> 
                  <td colspan=3 height="51"> 
                    <p>Type mailbox id or select from the list:</p>
                    <p> 
                      <input type="text" name="first" onkeyup="Si()">
                    </p>
                  </td>
                </tr>
                <tr> 
                  <td width="42%">System Voice Mail boxes</td>
                  <td width="18%">&nbsp;</td>
                  <td width="40%">Distribution List Members</td>
                </tr>
                <tr> 
                  <td> 
                    <select name="left" size="10" STYLE="width:180" multiple>
                      <option value="dpetrie@test.pingtel.com">dpetrie@test.pingtel.com</option>
                      <option value="hari@test.pingtel.com">hari@test.pingtel.com</option>
                      <option value="jcoffey@test.pingtel.com">jcoffey@test.pingtel.com</option>
                      <option value="jross@test.pingtel.com">jross@test.pingtel.com</option>
                      <option value="kdick@test.pingtel.com">kdick@test.pingtel.com</option>
                      <option value="rschaaf@test.pingtel.com">rschaaf@test.pingtel.com</option>
                    </select>
                  </td>
                  <td> 
                    <input type="button" name="Button" value=" &gt;&gt; " onclick="Ad()">
                    <br>
                    <br>
                    <input type="button" name="Submit2" value=" &lt;&lt; " onclick="Rm()">
                  </td>
                  <td> 
                    <select name="right" size="10" STYLE="width:180">
                    </select>
                  </td>
                </tr>
              </table>
            </td></tr>
		  <tr> 
			<td colspan="2" align="center">
              <input type="submit" name="submit3" value="Save List">
            </td>
		  </tr>
		</table>
		
	</td>
	</tr>
</table>
</form>
</body>
</html>
