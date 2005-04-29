<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.*,
                 com.pingtel.pds.common.PathLocatorUtil,
                 java.util.Properties"
%>
<%@ page isThreadSafe="false" %>
<%! String relaxSerialValidation = null;%>

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
<%
    String deviceID = request.getParameter("deviceid");
    String deviceTypeID = request.getParameter("devicetypeid");
    String mfgID = request.getParameter("mfgid");
    String userType = request.getParameter("usertype");
%>

<html>
    <head>
        <meta http-equiv="Pragma" content="no-cache">
        <meta http-equiv="expires" content="-1">
        <script src="../script/containers.js"></script>
        <script src="../script/jsDetails.js"></script>
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();
            var relaxSerialValidation = '<%=relaxSerialValidation%>';
        </script>
        <title>Device Details</title>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
    </head>
    <frameset rows="100,*,0" frameborder="NO" border="0" framespacing="0" cols="*" onunload="saveOnExit()">
        <frame name="topFrame"
        src="device_menu.jsp?deviceid=<%=deviceID%>&stylesheet=detailsmenu.xslt&mfgid=<%=mfgID%>&devicetypeid=<%=deviceTypeID%>&usertype=<%=userType%>">
        <frame name="mainFrame"
        scrolling="YES"
        noresize
        src="device_details.jsp?deviceid=<%=deviceID%>&stylesheet=details.xslt&refpropertygroupid=General&mfgid=<%=mfgID%>&devicetypeid=<%=deviceTypeID%>&usertype=<%=userType%>">
    </frameset>
    <noframes>
        <body bgcolor="#FFFFFF" text="#000000">
        </body>
    </noframes>
</html>
