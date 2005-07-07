<html>
    <head>
        <title>Manage Passwords</title>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
        <link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
        <script language="JavaScript">
            function formSubmit( form )
            {
                var formIsValid = true;
                if ( form.password.value != form.password2.value )
                {
                    formIsValid = false;
                    alert( "Your PIN fields do not match.\n" +
                    "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                    return;
                }
                else if ( isNaN ( form.password.value ) &&
                    form.password.value != "" )
                {
                    formIsValid = false;
                    alert( "Your PIN must be an integer.\n" +
                        "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                    return;
                }
                else
                {
                    form.submit();
                }
            }
        </script>
        <style type="text/css">
        <!--
        /* styling for error message */
        td#error { font: bold 100% Arial, Verdana, sans-serif;
            color: red;
            padding: 5px 0; }
        -->
        </style>
    </head>

<body bgcolor="#FFFFFF" text="#000000" class="bglight">


<table border="0" align="left" width="560">
<tr>
<td><img src="images/spacer.gif" width=5></td>
<td>
<table width="560" border="0">

<tr>

<td class="formtitle" height="30">Change PIN</td>
<td align="right"><a class="greentext" href="javascript:void 0" onclick="top.displayHelpPage('help/WebHelp/sipxchange.htm#password.htm');">Help</a></td>
</tr>
<tr>
<td class="formtitle" colspan="2">
<hr class="dms">
</td>
</tr>

<%
    String errorMessage = request.getParameter( "error_message" );

    if ( errorMessage != null ) {
        out.println( "<tr><td id='error'>" );
        out.println( errorMessage );
        out.println( "</td></tr>" );
    }
%>


<tr>
<td class="errortext_light" colspan="2">&nbsp; </td>
</tr>
<tr>
<td colspan="2">
<form method="post" action="set_users_password.jsp" onsubmit="formSubmit(this)">
<table border="0" class="bglist" align="left" cellpadding="4" cellspacing="1" width="560">
<tr>

<th colspan="2" class="tableheader">Change PIN</th>
</tr>
<tr>

<td class="tablecontents">Enter old PIN</td>
<td>
<input type="password" name="oldpassword">
</td>
</tr>
<tr>

<td class="tablecontents">Enter new PIN</td>
<td>
<input type="password" name="password">
</td>
</tr>
<tr>

<td class="tablecontents">Confirm new PIN</td>
<td>
<input type="password" name="password2">
</td>
</tr>
<tr>
<td colspan="2">
<div align="center">
<input type="hidden" name="userid" value='<%=request.getParameter("userid")%>'>
<input type="button" name="Submit" value="Save" onclick="formSubmit(this.form)">
</div>
</td>
</tr>
</table>
</form>
</td>
</tr>

<tr>
<td class="formtext" colspan="2">* Please note:  You will be logged out when you change your PIN.
&nbsp;Please log back in using your new PIN.</td>
</tr>

</table>
</td></tr></table>
</body>
</html>
