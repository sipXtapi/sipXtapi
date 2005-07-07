<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
    javax.ejb.*,
    java.rmi.RemoteException,
    javax.naming.*,
    javax.rmi.PortableRemoteObject,
    com.pingtel.pds.pgs.profile.*,
    java.util.*,
    java.lang.*,
    com.pingtel.pds.pgs.beans.ScriptletUtilityBean"
%>

<%@ taglib uri="pgstags" prefix="pgs" %>

<jsp:useBean id="commonUtility"
    class="com.pingtel.pds.pgs.beans.ScriptletUtilityBean"/>

<%
    String refconfigsetid = commonUtility.getCompleteDeviceRcsId();
%>

<%
    String groupName = request.getParameter( "groupname" );
    String parentGroupID = request.getParameter( "parentgroupid" );
%>
<html>
    <head>
        <title>Add Device Group</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();

            function formSubmit( form )
            {
                var formIsValid = true;
                if ( form.groupname.value == "" )
                {
                    formIsValid = false;
                    alert( "Please fill in the Device Group Name field" );
                }
                else if( !validator.isLegalName( form.groupname.value ) )
                {
                    var msg = "You have entered an invalid DeviceGroup Name.\n" +
                              "&,:,< and > are not allowed.\n" +
                              "Please try again.";
                    alert( msg );
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
            function reloadForm()
            {
                var form = document.frmCreateDeviceGroup;
                var groupName = "<%=groupName%>";
                var parentGroupID = "<%=parentGroupID%>";
                if ( groupName != "null" )
                {
                    form.groupname.value = groupName;
                    for ( var i = 0; i < form.parentgroupid.options.length; i++ )
                    {
                        if ( form.parentgroupid.options[i].value == parentGroupID )
                        {
                            form.parentgroupid.options[i].selected = true;
                        }
                    }
                }
            }
        </script>
    </head>
    <body class="bglight" onload="reloadForm()">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Add Device Group to the System</p>
        <table border="0" width="300" align="center">
        <tr><td>
        <font color=red>
        <%
            String errorMessage = request.getParameter( "error_message" );

            if ( errorMessage != null ) {
                out.println( errorMessage );
            }
        %>
        </font>
        </td></tr></table>
        <form action="../device/create_devicegroup.jsp" method="get" name="frmCreateDeviceGroup">
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap>Device Group Name *</td>
                    <td>
                        <input type="text" name="groupname" size="20">
                    </td>
                </tr>
                    <td class="formtext" nowrap>Parent Device Group *</td>
                    <td>
                        <select name="parentgroupid">
                            <option value="">&lt;BLANK&gt;</option>
                            <pgs:deviceGroupSelect stylesheet="groupselect.xslt" outputxml="false"/>
                        </select>
                     </td>
                </tr>
                <tr>
                    <td align="center" colspan="2">
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
                    <td align="left" class="formtext" colspan="2">* mandatory field</td>
                </tr>
            </table>

            <input type="hidden" name="refconfigsetid" value="<%=refconfigsetid%>">

        </form>
    </body>
</html>
