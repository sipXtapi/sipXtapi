<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
        javax.ejb.*,
        java.rmi.RemoteException,
        javax.naming.*,
        javax.rmi.PortableRemoteObject,
        com.pingtel.pds.pgs.profile.*,
        java.util.*,
        java.lang.*,
        java.io.*,
        com.pingtel.pds.common.PathLocatorUtil,
        com.pingtel.pds.common.EJBHomeFactory,
        com.pingtel.pds.pgs.beans.ScriptletUtilityBean"
%>
<%@ page isThreadSafe="false" %>
<%! String relaxSerialValidation = null;%>

<%@ taglib uri="pgstags" prefix="pgs" %>

<%
    if (relaxSerialValidation == null ) {
        String confPath =
            PathLocatorUtil.getInstance().getPath(  PathLocatorUtil.CONFIG_FOLDER,
                                                    PathLocatorUtil.PGS );

        Properties pgsProps = new Properties ();
        pgsProps.load( new FileInputStream (    confPath +
                                                    System.getProperty( "file.separator") +
                                                        "pgs.props" ) );

        String validateSerialNumber= pgsProps.getProperty(  "pgs.validate.device.serialnumber" );

        if ( validateSerialNumber != null &&
                validateSerialNumber.trim().equalsIgnoreCase("true" ) ) {
            relaxSerialValidation = "true";
        }
        else {
           relaxSerialValidation = "false";
        }
    }
%>
<jsp:useBean id="commonUtility"
    class="com.pingtel.pds.pgs.beans.ScriptletUtilityBean"/>

<%
    String refconfigsetid = commonUtility.getCompleteDeviceRcsId();
%>
<%
    //get values from url string
    String shortName = request.getParameter( "shortname" );
    String serialNumber = request.getParameter( "serialnumber" );
    String deviceTypeID = request.getParameter( "devicetypeid" );
    String deviceGroupID = request.getParameter( "devicegroupid" );
    String description = request.getParameter( "description" );

%>

<html>
    <head>
        <title>Add Device</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();
            var relaxSerialValidation = '<%=relaxSerialValidation%>';
            function formSubmit( form )
            {
                if ( form.shortname.value == "" )
                {
                    alert( "Please fill in the Device Name field" );
                }
                else if ( form.serialnumber.value == "" )
                {
                    alert( "Please fill in the Serial Number field" );
                }
                else if ( form.devicetypeid.value == "1" &&
							!validator.isSerialNumberXpressa(form.serialnumber.value) &&
                            relaxSerialValidation != "true" )
                {
                    var msg = "You have entered an invalid serial number.\n" +
                              "A valid serial number for a Xpressa device \n" +
							  "must start with 00d01e, and end with 6 characters, \n" +
                              "all of which must be either 0-9, a-f, or A-F, no spaces are allowed.\n" +
                              "Please try again.";
                    alert (msg);
					preLoadSerialNumber("1");
                }
				else if ( form.devicetypeid.value == "2" &&
							!validator.isSerialNumberIXpressa(form.serialnumber.value) &&
                            relaxSerialValidation != "true" )
                {
                    var msg = "You have entered an invalid serial number.\n" +
                              "A valid serial number for an Instant Xpressa device \n" +
							  "must start with ff0000, and end with 6 characters, \n" +
                              "all of which must be either 0-9, a-f, or A-F, no spaces are allowed.\n" +
                              "Please try again.";
                    alert (msg);
					preLoadSerialNumber("2");
                }
                else if ( ( form.devicetypeid.value != "1" ||
                            form.devicetypeid.value != "2" ) &&
							!validator.isSerialNumberOther(form.serialnumber.value) &&
                            relaxSerialValidation != "true" )
                {
                    var msg = "You have entered an invalid serial number.\n" +
                              "A valid serial number for this device \n" +
							  "must have 12 characters, \n" +
                              "all of which must be either 0-9, a-f, or A-F, no spaces are allowed.\n" +
                              "Please try again.";
                    alert (msg);
					form.serialnumber.value = "";
                }
                else if ( form.devicetypeid.value == "" )
                {
                    alert( "Please select a Device Type" );
                }
                else if ( form.devicegroupid.value == "" )
                {
                    alert( "Please select a Device Group" );
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
                var form = document.frmCreateDevice;
                var shortName = "<%=shortName%>";
                var serialNumber = "<%=serialNumber%>";
                var deviceTypeID = "<%=deviceTypeID%>";
                var deviceGroupID = "<%=deviceGroupID%>";
                var description = "<%=description%>";
                if ( shortName != "null" )
                {
                    form.shortname.value = shortName;
                    form.serialnumber.value = serialNumber;
                    form.description.value = description;
                    for ( var i = 0; i < form.devicetypeid.options.length; i++ )
                    {
                        if ( form.devicetypeid.options[i].value == deviceTypeID )
                        {
                            form.devicetypeid.options[i].selected = true;
                            if ( relaxSerialValidation != "true" ) {
                                form.serialnumber.disabled = false;
                            }
                        }
                    }
                    for ( var i = 0; i < form.devicegroupid.options.length; i++ )
                    {
                        if ( form.devicegroupid.options[i].value == deviceGroupID )
                        {
                            form.devicegroupid.options[i].selected = true;
                        }
                    }
                }
            }
			function preLoadSerialNumber( deviceType )
			{

                var form = document.frmCreateDevice;
                form.serialnumber.disabled = false;
                if ( relaxSerialValidation != "true" ) {
                    if( deviceType == "1" ) {
                        form.serialnumber.value = "00d01e";
                    }
                    else if( deviceType == "2" ) {
                        form.serialnumber.value = "ff0000";
                    }
                    else {
                        form.serialnumber.value = "";
                    }
                }
                else {
                    form.serialnumber.value = "";
                }
			}
        </script>
    </head>
    <body class="bglight" onload="reloadForm()">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Add Device to the System</p>
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
        <form action="../device/create_device.jsp" method="get" name="frmCreateDevice">
        <!-- <form action="../../test/test.jsp" method="get" name="frmCreateDevice"> -->
            <table width="300" border="0" cellspacing="3" cellpadding="4" align="center">
                <tr>
                    <td class="formtext" nowrap>Device Name *</td>
                    <td>
                        <input type="text" name="shortname">
                    </td>
                </tr>
                <tr>
                    <td class="formtext">Device Type *</td>
                    <td><pgs:deviceTypeSelect refProp="false" coreVersion="false"/></td>
                </tr>
				<tr>
                    <td class="formtext" nowrap>Serial Number *</td>
                    <td>
                        <input type="text" name="serialnumber" value="Select a Device Type" disabled>
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Description</td>
                    <td>
                        <textarea name="description" cols="20" rows="4"></textarea>
                    </td>
                </tr>

                <tr>
                    <td class="formtext" nowrap>Device Group *</td>
                    <td>
                        <select name="devicegroupid">
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
