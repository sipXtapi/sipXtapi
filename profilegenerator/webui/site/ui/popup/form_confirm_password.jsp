<%@ page import="java.io.*, java.util.*, com.pingtel.pds.pgs.common.*, java.net.*" %>

<%
      String password = request.getParameter( "password" );
%>

<html>
    <head>
        <script language="JavaScript">
            function formSubmit( form )
            {
                var formIsValid = true;
                if ( form.password.value != form.password2.value )
                {
                    formIsValid = false;
                    alert( "Your password fields do not match.\n" +
                    "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                }
                else
                {
                   var targetForm = self.opener.top.mainFrame.mainFrame.document.form1;
                   targetForm.elements["general.Password"].value = form.password.value;
                   this.window.close();
                }
            }
        </script>
        <title>Enter and Confirm Password</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight">
        <p>&nbsp;</p>

        <form>
            <table width="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td class="formtext" nowrap>Password</td>
                    <td>
                        <input type="password" name="password" value="<%=password%>">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Confirm Password</td>
                    <td>
                        <input type="password" name="password2">
                    </td>
                </tr>
            </table>
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
            </div>
        </form>
    </body>
</html>