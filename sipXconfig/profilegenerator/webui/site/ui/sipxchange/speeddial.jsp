<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
<head>
<link rel="stylesheet" type="text/css" href="style/dms_sipxchange.css">
<title>DeviceDetails</title>
<script language="JavaScript">
var saveFlag = false;
function setSaveFlag() {
    saveFlag = true;
}
function checkSaveFlag() {
    var msg = "Unsaved changes! To save your changes, click OK \n"  +
        "(Please note that if you are closing the browser, \n" +
        " your changes will not be saved)";
    if( saveFlag == true ) {
        if( confirm(msg) ) {
            document.form1.submit();
        }
    }
}

function formSubmit() {
    saveFlag = false;
    document.form1.submit();
}

function add_speeddial( sdid,label,type,address,rpid,replace,oldID )
{
    saveFlag = true;
    var date = new Date();
    var id = date.getTime();
    var displayAddress = address.replace(/</,"&lt;");
    displayAddress = displayAddress.replace(/>/,"&gt;");
    var speeddial = document.createElement("div");
    speeddial.setAttribute("id", id);

    if ( replace == "false" )
    {
        document.getElementById("insert_speeddial").appendChild(speeddial);
    }
    else
    {
        document.getElementById("insert_speeddial").replaceChild(
            speeddial, document.getElementById( oldID ));
    }

    speeddial.innerHTML = "<div id='display'><table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                          "<tr>" +
                          "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                          "</th>" +
                          "<th align='right'>" +
                          "<a href='#'><img onclick='parent.edit_speeddial(" + id + "," + rpid + ")'" +
                          "src='../../image/editicon.gif' alt='edit' width='12' height='12' alt='' border='0'></a>" +
                          "<img src='../../image/1x1green.gif' width='10'/>" +
                          "<a href='#'><img src='../../image/del.gif'" +
                          "onclick='parent.del_speeddial(" + id + ")' alt='delete' width='12' height='12' alt='' border='0'></a>" +
                          "</th>" +
                          "</tr>" +
                          "</table>" +
                          "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                          "<tr>" +
                          "<td colspan='3'>Speed dial number" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          sdid +
                          "</td></tr>" +
                          "<tr>" +
                          "<td colspan='3'>Name" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          label +
                          "</td></tr>"  +
                          "<tr>" +
                          "<td colspan='3'>Type" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          type +
                          "</td></tr>"  +
                          "<tr>" +
                          "<td colspan='3'>Number/URL" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          displayAddress +
                          "</td></tr></table></div>" +
                          "<input type=\"hidden\" value=\"" + sdid + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.ID.i_true.\" id=\"speeddial_sdid\">" +
                          "<input type=\"hidden\" value=\"" + label + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.LABEL.\" id=\"speeddial_label\">" +
                          "<input type=\"hidden\" value=\"" + type + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.TYPE.\" id=\"speeddial_type\">" +
                          "<input type=\"hidden\" value=\"" + address + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.ADDRESS.\" id=\"speeddial_address\">";
}

function speedDialVis()
{
    var speedDialHeadStyle = document.getElementById( "xp_2030head" ).style.display;
    var showHide = document.getElementById( "showhide" );
    var speedDialMsg = document.getElementById( "msg" );
    if ( speedDialHeadStyle == "none" )
    {
        showHide.style.display = "none";
        speedDialMsg.style.display = '';

    }
    else
    {
        showHide.style.display = '';
        speedDialMsg.style.display = "none";
    }
}

function loadTab( ) {
    document.getElementById("tab_Speed Dial").style.display = '';
}

</script>
</head>
<body class="bglight" onload="loadTab(); parent.checkMfg()" onunload="checkSaveFlag()">
<!--
<div id="msg">
    <p class="formtitle"><font color="red">The setting of Speed Dial numbers via the User UI is currently not available.  Please contact your system administrator to have
        this feature activated.
        </font></p>
</div> -->
<div id="msg1" style="display: none" class="readonly">The settings on this page apply only to your Pingtel phone(s).</div>
<div id="showhide">
<form action="../xml/update_configuration_set.jsp" method="post" name="form1">
<table width="600" border="0">
<tr>
		<td width="10"><img src="images/spacer.gif" width="10"></td>
		<td width="100%">
		<table width="100%" border="0">
    <tr>
      <td class="formtitle" height="30">&nbsp;Manage Speed Dial</td>
      <td height="30" align="right"><a class="greentext" href="javascript:void 0" onclick="top.displayHelpPage('help/WebHelp/sipxchange.htm#speeddial.html');">Help</a></td>
    </tr>
    <tr>
      <td class="errortext_light" colspan="2">
        <hr class="dms">
      </td>
    </tr>
    <tr>
      <td colspan=2>&nbsp;</td>
    </tr>
    <tr>
      <td class="list" colspan="2">
			<pgs:userDetails stylesheet='<%= request.getParameter("stylesheet")%>' outputxml="false"
                    userid='<%= request.getParameter("userid")%>'
                    refpropertygroupid="Speed Dial"
                    mfgid='<%=request.getParameter("mfgid")%>'
                    devicetypeid='<%=request.getParameter("devicetypeid")%>'
                    usertype='<%=request.getParameter("usertype")%>'
                    level="leaf"
                    visiblefor="user"
                    detailstype="user"/>
	</td>
   </tr>
	<tr>
      <td class="list" colspan="2">
		<hr class="dms" width="600" align="left">
		</td>
	</tr>
	<tr>
            <td colspan="2" align="center">
		<input type="button" value="Save" onclick="formSubmit()">
            </td>
	<tr>
</table>
</td>
</tr>
</table>
<input type="hidden" name="ppuser" value="2">
<input type="hidden" name="etype" value="user">
<input type="hidden" name="eid" value='<%=request.getParameter("userid")%>'>
<input type="hidden" name="x_entity_type" value="user">
<input type="hidden" name="x_entity_id" value='<%=request.getParameter("userid")%>'>
<input type="hidden" name="redir" value="../sipxchange/speeddial.jsp?userid=<%=request.getParameter("userid")%>&stylesheet=details.xslt&mfgid=pingtel&devicetypeid=common&usertype=user">
</form>
</div>
</body>
</html>
