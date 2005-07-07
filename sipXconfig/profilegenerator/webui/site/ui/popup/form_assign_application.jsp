<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <title>Untitled</title>
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
                this.window.parent.close();
            }
        </script>
    </head>
    <body class="bglight">

        <p class="formtitle" align="center">Assign Application to User</p>
        <form action="../user/assign_application.jsp" method="get" name="frmAssignApplication">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listUsersAvailApplications
                            stylesheet="assignapplications.xslt"
                            userid='<%=request.getParameter( "userid" )%>'
                            outputxml="false" />
                    </td>
                </tr>
            </table>
            <input type="Hidden" value='<%=request.getParameter( "userid" )%>' name="userid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Assign" onclick="formSubmit(this.form)">
            </div>
        </form>
        <hr class="dms">
        <p class="formtitle" align="center">Unassign Application from User</p>
        <form action="../user/unassign_application.jsp" method="get" name="frmUnassignApplication">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listUsersApplications userid='<%=request.getParameter( "userid" )%>'
                            stylesheet="unassignapplications.xslt"
                            outputxml="false"/>
                    </td>
                </tr>
            </table>

            <input type="Hidden" value='<%=request.getParameter( "userid" )%>' name="userid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Unassign" onclick="formSubmit(this.form)">
            </div>
            </form>
            <hr class="dms">
            <p class="formtitle" align="center">Currently Assigned Applications</p>
            <table border="0" cellspacing="3" cellpadding="3" align="center">
                <tr>
                    <th class="formtext">Name</th>
                </tr>
                <pgs:listUsersApplications userid='<%=request.getParameter( "userid" )%>'
                    stylesheet="listassignedapplication.xslt"
                    outputxml="false"/>
            </table>
         <br>
         <br>
         <div align="center">
            <input type="button" value="Close" onclick="closeWindow()">
        </div>
    </body>
</html>
