<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page language="Java" %>
<%@ page import="com.pingtel.pds.pgs.beans.*" %>

<html>
<head>
    <title>Add Extension Pool</title>
    <script src="../script/validate.js">
    </script>
    <script language="JavaScript">
        var validator = new Validate();

        function formSubmit( form )
        {
            if( form.name.value == "" )
            {
                alert ( "Please fill in the Name field" );
            }
            else
            {
                form.name.value = validator.trim( form.name.value );
                form.submit();
            }
        }
        function closeWindow()
        {
            self.opener.parent.mainFrame.location.reload();
            this.window.close();
        }

    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
</head>
    <body class="bglight">
    <p class="formtitle" align="center">
    Add Extension Pool
    </p>

    <jsp:useBean id="CreateExtPoolBean" class="com.pingtel.pds.pgs.beans.CreateExtensionPoolBean"
                scope="request" />

    <font color=red><jsp:getProperty name="CreateExtPoolBean" property="errorMessage"/></font>

    <form name="frmAddExtPool" method="post" action="../user/create_extpool.jsp">
        <div id="form">

            <table width="150" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext">Extension Pool Name *</td>
                    <td>
                        <input type="text" name="name" size="40"
                            value="<jsp:getProperty name="CreateExtPoolBean" property="name"/>">
                    </td>
                </tr>
                <tr>
                    <td colspan="3" align="center">
                        <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
                        &nbsp;&nbsp;
                        <input type="button" name="cmdCloseWindow" value="Close" onclick="closeWindow()"/>
                    </td>
                </tr>
                <tr>
                    <td align="left" class="formtext" colspan="2">* mandatory field</td>
                </tr>
            </table>
        </div>
    </form>
    </body>
</html>
