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
<%
    //get values from url string
    String displayID = request.getParameter( "displayid" );
    String firstName = request.getParameter( "firstname" );
    String lastName = request.getParameter( "lastname" );
    String extensionNumber = request.getParameter( "extensionnumber" );
    String userGroupID = request.getParameter( "usergroupid" );
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

<html>
    <head>
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();

            function formSubmit( form )
            {
                if ( form.displayid.value == "" )
                {
                        alert("Please fill in the User ID field");
                }
                else if( !validator.isUserId( form.displayid.value ) )
                {
                    var msg = "You have entered an invalid User ID.\n" +
                              "Only alphanumeric and _ characters are allowed.\n" +
                              "Please try again.";
                    alert( msg );
                }
                /* else if( !validator.isLegalName( form.firstname.value ) )
                {
                    var msg = "You have entered an invalid First Name.\n" +
                              "&,:,< and > are not allowed.\n" +
                              "Please try again.";
                    alert( msg );
                }
                else if( !validator.isLegalName( form.lastname.value ) )
                {
                    var msg = "You have entered an invalid Last Name.\n" +
                              "&,:,< and > are not allowed.\n" +
                              "Please try again.";
                    alert( msg );
                } */
                else if ( form.password.value != form.password2.value )
                {
                    formIsValid = false;
                    alert( "Your password fields do not match.\n" +
                        "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                }
                else if ( isNaN ( form.password.value ) &&
                    form.password.value != "" )
                {
                    formIsValid = false;
                    alert( "Your PIN must be an integer.\n" +
                        "Please try again." );
                    form.password.value = "";
                    form.password2.value = "";
                    return;
                }
                else if ( form.usergroupid.value == "" )
                {
                    alert( "Please select a User Group" );
                }
                else if ( form.extensionnumber.value != "" && isNaN(
                    form.extensionnumber.value ) )
                {
                    alert( "The extension field must be a number.\n" +
                        "Please try again." );
                }
                else
                {
                    form.submit();
                }
            }
            function nextExt( next )
            {
                var form = document.frmCreateUser;
                form.extensionnumber.value = next;
                form.extnumber.value = next;
            }

            function closeWindow()
            {
                self.opener.parent.mainFrame.location.reload();
                this.window.close();
            }
            function reloadForm()
            {
                var form = document.frmCreateUser;
                var displayID = "<%=displayID%>";
                var firstName = "<%=firstName%>";
                var lastName = "<%=lastName%>";
                var extensionNumber = "<%=extensionNumber%>";
                var userGroupID = "<%=userGroupID%>";
                if ( displayID != "null" )
                {
                    form.displayid.value = displayID;
                    form.firstname.value = firstName;
                    form.lastname.value = lastName;
                    form.extensionnumber.value = extensionNumber;
                    form.extnumber.value = extensionNumber;
                    for ( var i = 0; i < form.usergroupid.options.length; i++ )
                    {
                        if ( form.usergroupid.options[i].value == userGroupID )
                        {
                            form.usergroupid.options[i].selected = true;
                        }
                    }
                 }
            }
            function radioSelect( rb )
            {
                var rbValue = rb.value;
                var form = document.frmCreateUser;

                if ( rbValue == "1" ){
                form.extnumber.disabled = false;
                form.nextext.disabled = true;
                }
                else if ( rbValue == "2" ) {
                form.extnumber.disabled = true;
                form.nextext.disabled = false;
                }
            }
            function loadExtension()
            {
                var form = document.frmCreateUser;
                form.extensionnumber.value = form.extnumber.value;
            }
        </script>
        <title>Add User</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight" onload="reloadForm();document.frmCreateUser.displayid.focus()">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Add User to the System</p>
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

        <form action="../user/create_user.jsp" method="get" name="frmCreateUser">
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap>User ID *</td>
                    <td>
                        <input type="text" name="displayid">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>First Name</td>
                    <td>
                        <input type="text" name="firstname">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Last Name</td>
                    <td>
                        <input type="text" name="lastname">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Extension</td><td>&nbsp;</td>
                </tr>
                <tr>
                    <td class="formtext" align="right">Enter extension</td>
                    <td>
                        <input type="radio" name="rb1" value="1" onclick="radioSelect(this)" checked><input type="text" name="extnumber" onchange="loadExtension()">
                    </td>
                </tr>
                <tr>

                    <td class="formtext" align="right">Select from pool</td>
                    <td>
                        <input type="radio" name="rb1" value="2" onclick="radioSelect(this)"><pgs:listExtensionPools
                                                                                                organizationid='<%=session.getAttribute("orgid").toString()%>'
                                                                                                stylesheet="nextextension.xslt"
                                                                                                outputxml="false" />
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>PIN</td>
                    <td>
                        <input type="password" name="password">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Confirm PIN</td>
                    <td>
                        <input type="password" name="password2">
                    </td>
                </tr>
                <tr>
                    <td class="formtext">User Group *</td>
                    <td>
                        <select name="usergroupid">
                            <pgs:listUsers  stylesheet="groupselect.xslt"
                                outputxml="false"/>
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
            <br>
            <input type="hidden" name="refconfigsetid" value="<%=refconfigsetid%>">
            <input type="hidden" name="extensionnumber">
        </form>
    </body>
</html>
