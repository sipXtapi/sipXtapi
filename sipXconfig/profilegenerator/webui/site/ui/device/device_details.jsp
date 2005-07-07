<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<script language="javascript">
    function loadGroupSelect()
        {
            var currentValue = document.update.selectedgroupid.value;
            // replace the innerHTML
            if( document.getElementById("groupid") != null ) {
                document.getElementById("groupid").innerHTML =
                    document.getElementById("groupselect").innerHTML;

            // set current value to true
                var groupSelectBox = document.update.elements["general.DeviceGroupID"];
                for( var i = 0; i < groupSelectBox.options.length; i++ ) {
                    if( groupSelectBox.options[i].value == currentValue ) {
                        groupSelectBox.options[i].selected = true;
                    }
                }
            }
        }

</script>
<title>DeviceDetails</title>
</head>

<body class="bglight" onload="loadGroupSelect();parent.loadTab()">
<form name="update" method="post" action="../xml/update_configuration_set.jsp">
<pgs:deviceDetails stylesheet='<%= request.getParameter("stylesheet")%>' outputxml="false"
                    deviceid='<%= request.getParameter("deviceid")%>'
                    refpropertygroupid='<%= request.getParameter("refpropertygroupid")%>'
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    visiblefor="device"
                    level="leaf"
                    detailstype="device"/>
<input type="hidden" name="x_entity_type" value="device"/>
<input type="hidden" name="x_entity_id" value='<%=request.getParameter("deviceid")%>'/>
<input type="hidden" name="redir" value='<%=request.getRequestURI() + "?" + request.getQueryString()%>'/>
</form>
<div id="groupselect" style="display:none">
	<select name="general.DeviceGroupID" size="3" onchange="parent.setSaveFlag()">
		<pgs:deviceGroupSelect  stylesheet="groupselect.xslt"
			outputxml="false"/>
	</select>
</div>
</body>
</html>
