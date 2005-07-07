<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <title>Assign Application to Application Set</title>
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
                this.window.close();
            }
        </script>
    </head>
    <body class="bglight">

        <p class="formtitle" align="center">Assign Application to Application Set</p>
        <form action="../application/assign_application.jsp" method="get" name="frmAssignApplication">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listAppSetsAvailApplications
                            stylesheet="assignapplications.xslt"
                            applicationsetid='<%=request.getParameter( "applicationsetid" )%>'
                            outputxml="false" />
                    </td>
                </tr>
            </table>
            <input type="Hidden" value='<%=request.getParameter( "applicationsetid" )%>' name="applicationsetid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Assign" onclick="formSubmit(this.form)">
            </div>
        </form>
        <hr class="dms">
        <p class="formtitle" align="center">Unassign Application from Application Set</p>
        <form action="../application/unassign_application.jsp" method="get" name="frmUnassignApplication">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listApplicationSetsApplications applicationsetid='<%=request.getParameter( "applicationsetid" )%>'
                            stylesheet="unassignapplications.xslt"
                            outputxml="false"/>
                    </td>
                </tr>
            </table>

            <input type="Hidden" value='<%=request.getParameter( "applicationsetid" )%>' name="applicationsetid">
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
                <!-- added comment to change file -->
                <pgs:listApplicationSetsApplications applicationsetid='<%=request.getParameter( "applicationsetid" )%>'
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