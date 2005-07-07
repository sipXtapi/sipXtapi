<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>DeviceGroupDetails</title>
<script language="javascript">
function loadGroupSelect()
	{
		var currentValue = document.update.selectedpid.value;
        // replace the innerHTML
        if( document.getElementById("pid") != null ) {
            document.getElementById("pid").innerHTML =
                document.getElementById("groupselect").innerHTML;

        // set current value to true
            var groupSelectBox = document.update.elements["general.ParentID"];
            for( var i = 0; i < groupSelectBox.options.length; i++ ) {
                if( groupSelectBox.options[i].value == currentValue ) {
                    groupSelectBox.options[i].selected = true;
                }
            }
        }
    }

</script>
</head>

<body class="bglight" onload="parent.loadTab();loadGroupSelect()">
<form name="update" method="post" action="../xml/update_configuration_set.jsp">
<pgs:deviceGroupDetails stylesheet='<%= request.getParameter("stylesheet")%>' outputxml="false"
                    devicegroupid='<%= request.getParameter("devicegroupid")%>'
                    refpropertygroupid='<%= request.getParameter("refpropertygroupid")%>'
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    visiblefor="device"
                    level="group"
                    detailstype="devicegroup"/>
<input type="hidden" name="x_entity_type" value="devicegroup"/>
<input type="hidden" name="x_entity_id" value='<%=request.getParameter("devicegroupid")%>'/>
<input type="hidden" name="redir" value='<%=request.getRequestURI() + "?" + request.getQueryString()%>'/>
</form>
<div id="groupselect" style="display:none">
	<select name="general.ParentID" size="3" onchange="parent.setSaveFlag()">
    <option value="">&lt;BLANK&gt;</option>
		<pgs:deviceGroupSelect  stylesheet="groupselect.xslt"
			    devicegroupid='<%= request.getParameter("devicegroupid")%>'
                outputxml="false"/>
	</select>
</div>
</body>
</html>
