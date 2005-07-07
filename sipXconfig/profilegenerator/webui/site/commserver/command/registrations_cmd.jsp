<%@ page import="com.pingtel.pds.pgs.sipxchange.SatelliteLocation,
                 com.pingtel.pds.pgs.sipxchange.SatelliteManager,
                 java.util.Iterator,
                 com.pingtel.pds.common.URLPinger"%>
<html>
<head>
	<title>Untitled</title>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
<script src="../script/jsFunctions.js"></script>
<script language="javascript">
    function changeLocation() {
        var host = document.selectHostFrm.selectHost.value;
        parent.mainFrame.location = "../registration_details.jsp?location=" + host + "&sort=uri&index=1&count=50&max=50";
    }

</script>
</head>

<body class="bglight"
    onLoad="MM_preloadImages('../../ui/command/buttons/refresh_btn_f2.gif','../../ui/command/buttons/cancel_btn_f2.gif')">
<form name="selectHostFrm">
<table cellspacing="1" cellpadding="4" border="0" width="480">
    <tr>
        <td><a href="../registration_details.jsp?location=<%=request.getParameter("location")%>&sort=uri&index=1&count=50&max=50" target="mainFrame" onMouseOut="MM_swapImgRestore()"  onMouseOver="MM_swapImage('refresh_btn','','../../ui/command/buttons/refresh_btn_f2.gif',1);" onFocus="if(this.blur)this.blur()"><img name="refresh_btn" src="../../ui/command/buttons/refresh_btn.gif" border="0"></a></td>
        <td align="right" class="formtext">Server:</td>
        <td align="right">

        <select name="selectHost">

 <%
    String hostLoc = null;
    Iterator iterator = SatelliteManager.getInstance().getAllLocations().iterator();
    while (iterator.hasNext()){
        SatelliteLocation location = ( SatelliteLocation ) iterator.next();
        hostLoc = location.getHostName();
        Iterator it = location.getComponents(2).iterator();
        if( it.hasNext() ) {
        %>
        <option value='<%=hostLoc%>'><%=hostLoc%>
        <%
        }
    }
%>

        </select>

            <a href="#" onMouseOut="MM_swapImgRestore()" onclick="changeLocation()" onMouseOver="MM_swapImage('sm_btn_ass','','../../ui/buttons/go_btn2_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                <img name="sm_btn_ass" src="../../ui/buttons/go_btn2.gif" width="23" height="17" border="0" ></img>
            </a>
    </td>
    </tr>
</table>
</form>
</body>
</html>
