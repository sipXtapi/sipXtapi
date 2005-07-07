<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper,
                 com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome,
                 com.pingtel.pds.common.EJBHomeFactory" %>
<%
    String extensionpoolid = request.getParameter("extensionpoolid");
    String minextension = request.getParameter("minextension");
    String maxextension = request.getParameter("maxextension");
    String inRange = "";
    boolean bValidateInRange = false;
    TagHandlerHelper m_tagHanderHelper = null;
%>
<%


    if ( m_tagHanderHelper == null ) {
        TagHandlerHelperHome thhm = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                            TagHandlerHelperHome.class,
                            "TagHandlerHelper" );

                m_tagHanderHelper = thhm.create();
    }
    bValidateInRange = m_tagHanderHelper.extensionInExistingPool(
            minextension,
            maxextension,
            extensionpoolid );

    if( bValidateInRange == true ) {
         inRange = "true";
    } else {
        inRange = "false";
    }

%>

<html>
<head>
<script language="javascript">
    function checkInRange() {
        var inRange = '<%=inRange%>';
        var min = '<%=minextension%>';
        var max = '<%=maxextension%>';
        var extpoolid = '<%=extensionpoolid%>';

        if( inRange == "true" ) {
            var msg = "The range you are trying to create\n" +
                "overlaps an existing extension range.\n" +
                "Would you like to continue?";
            if( confirm(msg) ) {
                location = "create_ext_range.jsp?extensionpoolid=" +
                    extpoolid + "&minextension=" +
                    min + "&maxextension=" +
                    max;
            } else {
                location = "/pds/ui/popup/add_extensions.jsp?extensionpoolid=" + extpoolid;
            }
        } else {
            location = "create_ext_range.jsp?extensionpoolid=" +
                    extpoolid + "&minextension=" +
                    min + "&maxextension=" +
                    max;
        }
    }

</script>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
</head>
<body onload="checkInRange()" class="bglight">

</body>
</html>