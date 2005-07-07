
<%@ page import="java.io.*, java.util.*, com.pingtel.pds.pgs.common.*, java.net.*" %>

<%
      String realm = request.getParameter( "REALM" );
      String userid = request.getParameter( "USERID" );
      String target = request.getParameter( "target" );
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
                    form.submit();
                }
            }
        </script>
        <title>Enter and Confirm Password</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight">
        <p>&nbsp;</p>

        <form action="/pds/ui/popup/digest.jsp" method="get" name="frmCalcDigest">
            <table width="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td class="formtext" nowrap>Password</td>
                    <td>
                        <input type="password" name="password">
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
            <input type="hidden" name="realm" value="<%=realm%>">
            <input type="hidden" name="userid" value="<%=userid%>">
            <input type="hidden" name="target" value="<%=target%>">
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
            </div>
        </form>
    </body>
</html>