<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<script language="javascript">
function reloadMain()
{
    parent.mainFrame.document.location.reload();
}
</script>
</head>
<body onload="reloadMain()">
<div id="groupselect" style="display:none">
	<select name="general.DeviceGroupID" size="3" onchange="parent.addInfo('onchange');parent.setDeviceSelectGroupFlag()">
		<pgs:listDevices  stylesheet="groupselect.xslt"
			outputxml="false"/>
	</select>
</div>
</body>
</html>