<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
    javax.ejb.*,
    java.rmi.RemoteException,
    javax.naming.*,
    javax.rmi.PortableRemoteObject,
    com.pingtel.pds.pgs.phone.*,
    com.pingtel.pds.pgs.profile.*,
    com.pingtel.pds.common.PDSDefinitions,
    java.util.*,
    java.lang.*,
    com.pingtel.pds.pgs.beans.*,
    com.pingtel.pds.common.EJBHomeFactory"
%>
<%@ taglib uri="pgstags" prefix="pgs" %>

<jsp:useBean id="commonUtility"
    class="com.pingtel.pds.pgs.beans.ScriptletUtilityBean"/>

<%
    String devicetypeid =
            commonUtility.getDeviceTypeId(
                    PDSDefinitions.MODEL_HARDPHONE_XPRESSA);

    String refpropertyid = commonUtility.getXpressaApplicationRefPropId();
%>

<html>
    <head>
        <title>Add Application</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script language="JavaScript">
            function formSubmit( form )
            {
                var formIsValid = true;
                if ( form.name.value == "" )
                {
                    alert( "Please fill the Name field" );
                }
                else if ( form.url.value == "" )
                {
                    alert( "Please fill in the URL field" );
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
        <p class="formtitle" align="center">Add Application to the System</p>

        <jsp:useBean id="CreateApplicationBean" class="com.pingtel.pds.pgs.beans.CreateApplicationBean"
                scope="request" />
        <font color=red><jsp:getProperty name="CreateApplicationBean" property="errorMessage"/></font>

        <form action="../application/create_application.jsp" method="get" name="frmCreateApplication">
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap valign="top" width="40%">Name *</td>
                    <td>
                        <input type="Text" name="name" size="40"
                            value="<jsp:getProperty name="CreateApplicationBean" property="name"/>"/>
                        </input>
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap valign="top" width="40%">URL *</td>
                    <td>
                        <input type="Text" name="url" size="40"
                            value="<jsp:getProperty name="CreateApplicationBean" property="url"/>">
                        </input>
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap valign="top" width="40%">Description</td>
                    <td>
                        <textarea cols="20" rows="5" name="description"
                            value="<jsp:getProperty name="CreateApplicationBean" property="description"/>">
                        </textarea>
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
                <tr><td align="left" colspan="2" class="formtext">* mandatory field</td></tr>
            </table>
            <br>
            <input type="hidden" name="devicetypeid" value="<%=devicetypeid%>">
            <input type="hidden" name="refpropertyid" value="<%=refpropertyid%>">

        </form>
    </body>
</html>
