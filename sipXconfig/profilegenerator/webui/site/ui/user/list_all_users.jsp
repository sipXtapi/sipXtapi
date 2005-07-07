<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String searchID = "";
    if ( request.getParameter("groupid") != null )
    {
        session.setAttribute("ugid", request.getParameter("groupid"));
    }
    else
    {
        session.setAttribute("ugid", "all");
    }
    if( request.getParameter("id") != null ){
        searchID = request.getParameter("id");
    }

%>
<html>
<head>
<title>ListUsers</title>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<script language="JavaScript">
<!--
function frmSubmit( id,grpid,displayid )
{
    var value = eval( "document.form1.actionSelect" + id + ".value" );
    if ( value == "1" )
    {
        sendProfiles( id );
    }
    else if ( value == "2" )
    {
       assignApplication( id );
    }
    else if ( value == "3" )
    {
       assignDevice( id,displayid );
    }
    else if ( value == "4" )
    {
      copyUser( id,grpid );
    }
    else if ( value == "5" )
    {
       deleteUser( id,grpid );
    }
    else if ( value == "6" )
    {
       restartDevices( id,grpid );
    }
    else
    {
        alert( "Please Select an Action Item" );
    }
}
function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments; document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2) eval(args[i]+".location='"+args[i+1]+"'");
}

function deleteUser( id,grpid ){
    var msg = "Are you sure you want to delete the user? ";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        location = "delete_user.jsp?userid=" + id +
            "&grpid=" + grpid;
    }
}

function copyUser( id,grpid ){
    var msg = "Are you sure you want to copy the user?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        location = "copy_user.jsp?sourceuserid=" + id +
            "&grpid=" + grpid;
    }
}
function assignDevice( id,displayid ){
	var targetWindow = "/pds/ui/popup/form_assign_device.jsp?userid=" + id + "&displayid=" + displayid;
	window.parent.MM_openBrWindow(targetWindow,'popup','scrollbars=yes,width=400,height=500');
}
function restartDevices( id,grpid ){
    var msg = "Are you sure you want to restart device(s)?";
    var userInput = confirm ( msg );
    if ( userInput == true )
    {
        location = "restart_user_devices.jsp?userid=" + id +
            "&grpid=" + grpid;
    }
}
function assignApplication( id ){
	var targetWindow = "/pds/ui/popup/frame_user_applications.jsp?userid=" + id;
	window.parent.MM_openBrWindow(targetWindow,'popup','scrollbars=yes,width=400,height=600');
}
function sendProfiles( id ){
    var targetWindow = "/pds/ui/popup/form_project_profiles.jsp?entityid=" + id + "&entitytype=user";
    window.parent.MM_openBrWindow(targetWindow,'popup','scrollbars=no,width=400,height=350');
}
function filterGroups(id)
{
    document.location = "list_all_users.jsp?groupid=" + id;
}
function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}
function checkSearch() {
    var id = '<%=searchID%>';
    if( id != "" ) {
        var found = document.getElementById( id );
		var anchor = document.getElementById( "anchor" + id );
		found.style.backgroundColor = '#99FFFF';
		anchor.focus();
	}
}
//-->
</script>
</head>
<body class="bglight"
	onLoad="MM_goToURL('parent.frames[\'command\']','../command/list_all_users_cmd.html');
    checkSearch();
	MM_preloadImages('../buttons/members_btnf2.gif');return document.MM_returnValue">
<table width="600">
    <tr>
        <td width="50%" align="left">
            <h1 class="list">Users</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#list_users.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>

<!-- <form name="filter">
    <table>
        <tr>
            <td class="formtext">Filter by:</td>
            <td>
                <pgs:listUsersGroupsFilter    stylesheet="filterusergroups.xslt"
                                    outputxml="false" />
            </td>
        </tr>
    </table>
</form> -->
<form name="form1">
<pgs:listUsers  stylesheet="listusers.xslt"
                outputxml="false"
                groupid='<%=request.getParameter( "groupid" )%>'/>
</form>
</body>
</html>
