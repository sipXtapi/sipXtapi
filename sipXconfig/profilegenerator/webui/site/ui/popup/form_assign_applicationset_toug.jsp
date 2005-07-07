<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <title>Assign Application Sets to User Group</title>
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

        <p class="formtitle" align="center">Assign Application Set to User Group</p>
        <form action="../user/assign_applicationset_to_ug.jsp" method="get" name="frmAssignApplicationSet">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listUserGroupsAvailApplicationSets
                            stylesheet="assignappsets.xslt"
                            usergroupid='<%=request.getParameter( "usergroupid" )%>'
                            outputxml="false" />
                    </td>
                </tr>
            </table>
            <input type="Hidden" value='<%=request.getParameter( "usergroupid" )%>' name="usergroupid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Assign" onclick="formSubmit(this.form)">
            </div>
        </form>
        <hr class="dms">
        <p class="formtitle" align="center">Unassign Application Set from User Group</p>
        <form action="../user/unassign_applicationset_from_ug.jsp" method="get" name="frmUnassignApplicationSet">
            <table border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td>
                        <pgs:listUserGroupsApplicationSets usergroupid='<%=request.getParameter( "usergroupid" )%>'
                            stylesheet="unassignappsets.xslt"
                            outputxml="false"/>
                    </td>
                </tr>
            </table>

            <input type="Hidden" value='<%=request.getParameter( "usergroupid" )%>' name="usergroupid">
            <br>
            <div align="center">
                <input type="button" name="cmdSubmitForm" value="Unassign" onclick="formSubmit(this.form)">
            </div>
            </form>
            <hr class="dms">
            <p class="formtitle" align="center">Currently Assigned Application Sets</p>
            <table border="0" cellspacing="3" cellpadding="3" align="center">
                <tr>
                    <td class="formtext" align="left">Name</th>
                </tr>
                <pgs:listUserGroupsApplicationSets usergroupid='<%=request.getParameter( "usergroupid" )%>'
                    stylesheet="listassignedappsets.xslt"
                    outputxml="false"/>
            </table>
         <br>
         <br>
         <div align="center">
            <input type="button" value="Close" onclick="closeWindow()">
        </div>
    </body>
</html>
