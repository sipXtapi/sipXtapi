<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
	<title>Assign Device to User</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script language="JavaScript">
            function formSubmit( form )
            {
                if ( form.elements[0].selectedIndex == -1 ||
                        form.elements[0].selectedIndex == 0 )
                {}
                else
                {
                    form.submit();
                }
            }

            function reloadMain()
            {
                self.opener.parent.mainFrame.location.reload();
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
        <p class="formtitle" align="center">Assign Device to User: <%=request.getParameter( "displayid" )%></p>
        <form action="../user/assign_device.jsp" method="get" name="frmAssignDevice">
            <table width="300" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td align="center">
                        <pgs:availDevices />
                    </td>
                </tr>
            </table>
            <input type="Hidden" value='<%=request.getParameter( "userid" )%>' name="userid">
            <input type="Hidden" value='<%=request.getParameter( "displayid" )%>' name="displayid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Assign" onclick="formSubmit(this.form)">
            </div>
        </form>
        <hr>
        <p class="formtitle" align="center">Unassign Device from User: <%=request.getParameter( "displayid" )%></p>
        <form action="../user/unassign_device.jsp" method="get" name="frmUnassignDevice">
            <table width="300" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td align="center">
                        <pgs:assignedDevices userid='<%=request.getParameter( "userid" )%>'/>
                    </td>
                </tr>
            </table>
            <input type="Hidden" value='<%=request.getParameter( "userid" )%>' name="userid">
            <input type="Hidden" value='<%=request.getParameter( "displayid" )%>' name="displayid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Unassign" onclick="formSubmit(this.form)">
            </div>
            <br>
            <br>
            <div align="center">
                <input type="button" value="Close" onclick="closeWindow()">
            </div>
       </form>
    </body>
</html>
