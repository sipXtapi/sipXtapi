<%
    String dnsdomain = request.getParameter("dnsdomain");
    String currentAlias = request.getParameter("id");
%>
<html>
<head>
    <title>Edit Alias</title>
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
                var oldID = form.oldid.value;
                this.opener.top.mainFrame.add_alias(alias, "true", oldID, '<%=dnsdomain%>' );
                this.window.close();
            }
        }
        function closeWindow()
        {
            this.window.close();
        }

    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    <style type="text/css">
    <!--
    div#form {text-align: center;}
        td#introtext {font: bold 100% Arial, Helvetica, Verdana, sans-serif;
    color: #669999; text-align: right; padding-right: 5px; }
        td#exittext {font: bold 100% Arial, Helvetica, Verdana, sans-serif;
    color: #669999; text-align: left; padding-left: 5px; }
        td#buttons {text-align: center; padding: 10px 0;}
    -->
    </style>
</head>
    <body class="bglight">
    <p class="formtitle">
    Edit an Alias
    </p>
    <form name="frmEditAlias" onsubmit="formSubmit(this)">
        <div id="form">

            <table idth="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td id="introtext">sip:</td>
                    <td id="field">
                        <input type="text" name="alias" id="alias" size="40" value='<%=currentAlias%>'>
                    </td>
                    <td id="exittext" nowrap>@<%=dnsdomain%></td>
                </tr>
                <tr>
                    <td colspan="3" id="buttons">
                        <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
                        &nbsp;&nbsp;
                        <input type="button" name="cmdCloseWindow" value="Cancel" onclick="closeWindow()"/>
                    </td>
                </tr>
            </table>
        </div>
        <input type="hidden" name="oldid" value='<%=currentAlias%>'>
    </form>
    </body>
</html>
