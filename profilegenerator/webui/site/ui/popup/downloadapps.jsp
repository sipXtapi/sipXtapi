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
                 com.pingtel.pds.pgs.common.PGSDefinitions,
                 com.pingtel.pds.common.PathLocatorUtil,
                 java.io.FileInputStream,
                 com.pingtel.pds.common.RedirectServletException,
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
<title>Download Applications</title>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<script language="JavaScript">
<!--
function setJavaParams() {
var deviceTypeID = '<%=devicetypeid%>';
var refPropertyID = '<%=refpropertyid%>';
var rootDoc = document.getElementsByTagName("*");
for( var i = 0; i < rootDoc.length; i++ ) {
if( rootDoc[i].nodeName == 'INPUT' ) {
if(  rootDoc[i].id.indexOf("did" ) != -1 ) {
rootDoc[i].value = deviceTypeID;
}
else if( rootDoc[i].id.indexOf("rpid") != -1 ) {
rootDoc[i].value = refPropertyID;
}
}
}
}

function checkLast() {
if( top.location.search != "" ) {
var query = top.location.search.substring(1);
var cat = query.substring( query.indexOf("=") + 1 );
var docRoot = document.getElementsByTagName("*");
for ( var i = 0; i < docRoot.length; i++ ) {
if ( docRoot[i].nodeName == 'DIV' && docRoot[i].id.indexOf( "cat_" ) != -1 ) {
docRoot[i].style.display = 'none';
}
}
document.getElementById( decodeURI(cat) ).style.display = '';
}
}
//-->
</script>
</head>
<body class="bglight" onload="setJavaParams();checkLast()">
<font color=red>
    <%
        String errorMessage = request.getParameter("error_message");

        if (errorMessage != null) {
            out.println(errorMessage);
        }
    %>
    </font>
    <%
        String pgsPropertiesFileName =
                new String (PathLocatorUtil.getInstance().getPath(
                        PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS) +
                "pgs.props" );

        Properties pgsProperties = new Properties();
        pgsProperties.load (new FileInputStream (pgsPropertiesFileName));

        String applicationsDocRoot =
                pgsProperties.getProperty("publishedHttpApplicationsDocRoot");

        if (applicationsDocRoot == null){
    %>
    <font color=red>
    <%= ResourceBundle.getBundle("webui-resources").getString("E_NO_APP_ROOT") %>
    </font>
    <%
        } else {
    %>

    <pgs:getApplicationsForDownload
        sourcexmlurl='<%=PGSDefinitions.APPS_EXTERNAL_LOCATION%>'
        stylesheet="downloadapps.xslt"
        outputxml="false"/>

    <%
        }
    %>

</body>
</html>