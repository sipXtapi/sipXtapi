<%
    String dnsdomain = request.getParameter("dnsdomain");
%>
<html>
<head>
    <title>Add Alias</title>
    <script src="../script/validate.js">
    </script>
    <script language="JavaScript">
        var validator = new Validate();
        function formSubmit( form )
            {
                var nFlag = true;
                var inputs;
                var parent = String( this.opener.parent.location );
                inputs = this.opener.top.mainFrame.mainFrame.document.getElementsByTagName('INPUT');
                for ( i = 0; i < inputs.length; i++ )
                {
                    if ( inputs[i].id.indexOf( "alias." ) != -1 )
                    {
                        if ( inputs[i].value == form.alias.value )
                        {
                            nFlag = false;
                        }
                    }
                }
                if ( form.alias.value == "" )
                {
                    alert( "Please fill in the User Name field" );
                }
                else if ( nFlag == false )
                {
                    var msg = "The alias must be unique.\n" +
                              "Please enter a different alias. ";

                    alert( msg );
                    form.alias.value = "";
                }
                else if ( !validator.isAlias( validator.trim( form.alias.value ) ) )
                {
                    alert( "The value entered for an alias is not valid.\n" +
                        "Please do not use 'sip:' or @ in the alias field" );
                }
                else
                {
                    var alias = validator.trim(form.alias.value);
                    this.opener.top.mainFrame.add_alias(alias, "false", null, '<%=dnsdomain%>' );
                    this.window.close();
                }
            }
            function closeWindow()
            {
                this.window.close();
            }

    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
</head>
    <body class="bglight">
    <p class="formtitle">
    Add an Alias
    </p>
    <form name="frmAddAlias" onsubmit="formSubmit(this)">
        <div id="form">

            <table width="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td class="formtext" nowrap>sip:</td>
                    <td>
                        <input type="text" name="alias" id="alias" size="40">
                    </td>
                    <td class="formtext" nowrap>@<%=dnsdomain%></td>
                </tr>
                <tr>
                    <td colspan="3" align="center">
                        <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
                        &nbsp;&nbsp;
                        <input type="button" name="cmdCloseWindow" value="Cancel" onclick="closeWindow()"/>
                    </td>
                </tr>
            </table>
        </div>
    </form>
    </body>
</html>
