<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>UserGroupDetails</title>
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
<pgs:userGroupDetails stylesheet='<%= request.getParameter("stylesheet")%>' outputxml="false"
                    usergroupid='<%= request.getParameter("usergroupid")%>'
                    refpropertygroupid='<%= request.getParameter("refpropertygroupid")%>'
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    visiblefor="user"
                    level="group"
                    detailstype="usergroup"/>
<input type="hidden" name="x_entity_type" value="usergroup"/>
<input type="hidden" name="x_entity_id" value='<%=request.getParameter("usergroupid")%>'/>
<input type="hidden" name="redir" value='<%=request.getRequestURI() + "?" + request.getQueryString()%>'/>
</form>
<div id="groupselect" style="display:none">
	<select name="general.ParentID" size="3" onchange="parent.setSaveFlag()">
    <option value="">&lt;BLANK&gt;</option>
		<pgs:userGroupSelect  stylesheet="groupselect.xslt"
			    usergroupid='<%= request.getParameter("usergroupid")%>'
                outputxml="false"/>
	</select>
</div>
</body>
</html>
