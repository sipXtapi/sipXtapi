<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
    javax.ejb.*,
    java.rmi.RemoteException,
    javax.naming.*,
    javax.rmi.PortableRemoteObject,
    com.pingtel.pds.pgs.profile.*,
    java.util.*,
    java.lang.*"
%>
<%@ taglib uri="pgstags" prefix="pgs" %>

<%
    RefConfigurationSetHome refConfSetHome = null;
    String refconfigsetid = null;
    try
    {
        InitialContext initial = new InitialContext();
        Object objref = initial.lookup( "RefConfigurationSet" );
        refConfSetHome = ( RefConfigurationSetHome ) PortableRemoteObject.narrow(
            objref, RefConfigurationSetHome.class);

        Collection sets = refConfSetHome.findByName( "Complete User" );
        Iterator setsIterator = sets.iterator();
        com.pingtel.pds.pgs.profile.RefConfigurationSet refSetProp;
        while( setsIterator.hasNext() )
        {
            refSetProp = ( com.pingtel.pds.pgs.profile.RefConfigurationSet )
            setsIterator.next();
            refconfigsetid = refSetProp.getID().toString();
        }
    }
    catch ( RemoteException ioe ) {
        throw new JspTagException(
            "RemoteException: " + ioe.getMessage() );
    }
    catch ( NamingException ne ) {
        throw new JspTagException(
            "NamingException: " + ne.getMessage() );
    }
    catch ( FinderException fe ) {
        throw new JspTagException(
            "FinderException: " + fe.getMessage() );
    }
%>
<%
    String userGroupName = request.getParameter( "usergroupname" );
    String parentGroupID = request.getParameter( "parentgroupid" );
%>

<html>
    <head>
        <title>Add User Group</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();

            function formSubmit( form )
            {
                var formIsValid = true;
                if ( form.usergroupname.value == "" )
                {
                    formIsValid = false;
                    alert( "Please fill in the User Group Name field" );
                }
                else if( !validator.isLegalName( form.usergroupname.value ) )
                {
                    var msg = "You have entered an invalid UserGroup Name.\n" +
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
                var form = document.frmCreateUserGroup;
                var userGroupName = "<%=userGroupName%>";
                var parentGroupID = "<%=parentGroupID%>";
                if ( userGroupName != "null" )
                {
                    form.usergroupname.value = userGroupName;
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
        <p class="formtitle" align="center">Add User Group to the System</p>
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
        <form action="../user/create_usergroup.jsp" method="get" name="frmCreateUserGroup">
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap>User Group Name *</td>
                    <td>
                        <input type="text" name="usergroupname" size="20">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Parent User Group *</td>
                    <td>
                        <select name="parentgroupid">
                            <option value="">&lt;BLANK&gt;</option>
                            <pgs:userGroupSelect stylesheet="groupselect.xslt" outputxml="false"/>
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
