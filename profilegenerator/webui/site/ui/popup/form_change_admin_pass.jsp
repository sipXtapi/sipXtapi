
<%@ page import="java.io.*, java.util.*, com.pingtel.pds.pgs.common.*, java.net.*" %>

<%
      String userid = request.getParameter( "userid" );
%>

<html>
    <head>
        <script language="JavaScript">
            function formSubmit( form )
            {
                if ( form.password.value != form.password2.value )
                {
                    alert( "Your password fields do not match.\n" +
                    "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                }
                else if( form.password.value.length < 3 ||
                    form.password.value.length > 12 )
                {
                    alert( "Your password must be at least 3 characters,\n" +
                    "but not more than 12 characters long.\n" +
                    "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
				}
                else
                {
                    form.submit();
                }
            }
            function closeWindow()
            {
                this.window.close();
            }
        </script>
        <title>Change Admin Password</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight">
        <p>&nbsp;</p>

        <form action="../administration/changepassword.jsp" method="get" name="frmChangePassword">
            <table width="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td class="formtext" nowrap>Password</td>
                    <td>
                        <input type="password" name="password" size="12">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Confirm Password</td>
                    <td>
                        <input type="password" name="password2" size="12">
                    </td>
                </tr>
            </table>
            <br>
            <input type="hidden" name="userid" value="<%=userid%>">
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
                <input type="button" name="cmdClose" value="Close" onclick="closeWindow()">
            </div>
        </form>
    </body>
</html>
