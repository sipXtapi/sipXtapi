<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="com.pingtel.pds.pgs.beans.CreateApplicationSetBean" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <title>Add Application Set</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script language="JavaScript">
            function formSubmit( form )
            {
                var regExpToCheck = /^(u|ug)[0-9]+_/i;
				var formIsValid = true;
                if ( form.name.value == "" )
                {
                    alert( "Please fill the Name field" );
                }
                else if ( form.name.value.match( regExpToCheck ) )
                {
                    alert( "The name of your application set/n" +
							"may not start with u{digit(s)}_ or/n" +
							"ug{digit(s)}_. Please try again." );
					form.name.value = "";
                }
				else
				{
					form.submit();
				}
            }
            function closeWindow()
            {
                self.opener.parent.mainFrame.location.reload();
                this.window.close();
            }
        </script>
    </head>
    <body class="bglight">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Add Application Set to the System</p>

        <jsp:useBean id="CreateApplicationSetBean" class="com.pingtel.pds.pgs.beans.CreateApplicationSetBean"
                scope="request" />
        <font color=red><jsp:getProperty name="CreateApplicationSetBean" property="errorMessage"/></font>

        <form action="../application/create_applicationgroup.jsp" method="get" name="frmCreateAppSet">
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap>Application Set Name *</td>
                    <td>
                        <input type="text" name="name" size="20"
                            value="<jsp:getProperty name="CreateApplicationSetBean" property="name"/>" />
                    </td>
                <tr>
                    <td colspan="2" align="center">
                        <input type="button"
                            name="cmdSubmitForm"
                            value="Submit"
                            onclick="formSubmit(this.form)">
                        &nbsp;
                        <input type="button"
                            name="cmdCloseWindow"
                            value="Close"
                            onclick="closeWindow()">
                    </td>
                </tr>
                <tr>
                    <td colspan="2" align="left" class="formtext">* mandatory field</td>
                </tr>
            </table>
        </form>
    </body>
</html>
