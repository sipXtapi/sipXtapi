<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<script language="JavaScript">
    function radioSelect( rb )
    {
        var rbValue = rb.value;
        var form = document.update;

        if ( rbValue == "1" ){
        form.elements["general.Extension"].disabled = false;
        form.nextext.disabled = true;
        }
        else if ( rbValue == "2" ) {
        //form.elements["general.Extension"].disabled = true;
        form.nextext.disabled = false;
        }
    }
    function loadExtSelect()
    {
        if( document.getElementById("extpool") != null ) {
            document.getElementById("extpool").innerHTML =
                document.getElementById("extpoolselect").innerHTML;
        }
    }
    function nextExt( next ){
        document.update.elements["general.Extension"].value = next;
        parent.validateExt( next );
    }
    function loadGroupSelect()
	{
		var currentValue = document.update.selectedgroupid.value;
        // replace the innerHTML
        if( document.getElementById("groupid") != null ) {
            document.getElementById("groupid").innerHTML =
                document.getElementById("groupselect").innerHTML;

        // set current value to true
            var groupSelectBox = document.update.elements["general.UserGroupID"];
            for( var i = 0; i < groupSelectBox.options.length; i++ ) {
                if( groupSelectBox.options[i].value == currentValue ) {
                    groupSelectBox.options[i].selected = true;
                }
            }
        }
    }
</script>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<title>UserDetails</title>
</head>

<body class="bglight" onload="loadExtSelect();loadGroupSelect();parent.loadTab()">
<form name="update" method="post" action="../xml/update_configuration_set.jsp">
<pgs:userDetails stylesheet='<%= request.getParameter("stylesheet")%>' outputxml="false"
                    userid='<%= request.getParameter("userid")%>'
                    refpropertygroupid='<%= request.getParameter("refpropertygroupid")%>'
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    visiblefor="user"
                    level="leaf"
                    detailstype="user"/>


<input type="hidden" name="x_entity_type" value="user"/>
<input type="hidden" name="x_entity_id" value='<%=request.getParameter("userid")%>'/>
<input type="hidden" name="redir" value='<%=request.getRequestURI() + "?" + request.getQueryString()%>'/>
</form>
<div id="extpoolselect" style="display:none">
    <input type="radio" name="rb1" value="2" onclick="radioSelect(this)"><pgs:listExtensionPools
                                                                            organizationid='<%=session.getAttribute("orgid").toString()%>'
                                                                            stylesheet="nextextension.xslt"
                                                                            outputxml="false" />
</div>
<div id="groupselect" style="display:none">
	<select name="general.UserGroupID" size="3" onchange="parent.setSaveFlag()">
		<pgs:userGroupSelect  stylesheet="groupselect.xslt"
			outputxml="false"/>
	</select>
</div>
</body>
</html>
