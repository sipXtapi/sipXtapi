<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String orgid = session.getAttribute("orgid").toString();
%>
<html>
<head>
<title>List Extension Pools</title>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<script language="JavaScript">
<!--
function frmSubmit( id )
{
    var value = eval( "document.form1.actionSelect" + id + ".value" );
    if ( value == "1" )
    {
        addExtensions( id );
    }
    else if ( value == "2" )
    {
       delExtensions( id );
    }
    else if ( value == "3" )
    {
       deleteExtPool( id );
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

function deleteExtPool( id ){
    var msg = "Are you sure you want to delete this Extension Pool? ";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        location = "list_all_extpools.jsp?extensionpoolid=" + id + "&deletepool=true";
    }
}

function addExtensions( id ){
	var targetWindow = "/pds/ui/popup/add_extensions.jsp?extensionpoolid=" + id + "&submit_flag=false";
	window.parent.MM_openBrWindow(targetWindow,'popup','scrollbars=yes,width=400,height=300');
}
function delExtensions( id ){
	var targetWindow = "/pds/ui/popup/del_extensions.jsp?extensionpoolid=" + id +"&submit_flag=false";
	window.parent.MM_openBrWindow(targetWindow,'popup','scrollbars=yes,width=400,height=300');
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

//-->
</script>
</head>
<body class="bglight"
	onLoad="MM_goToURL('parent.frames[\'command\']','../command/list_all_extpools_cmd.html');
	MM_preloadImages('../buttons/members_btnf2.gif');return document.MM_returnValue">
<table width="600">
    <tr>
        <td width="50%" align="left">
            <h1 class="list">Extension Pools</h1>
        </td>
        <td width="50%" align="right">
            <a href="#" onclick="window.parent.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/ConfigSrvr.htm#extension_pools.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a>
        </td>
    </tr>
    <tr>
        <td colspan="2"><hr class="dms"></td>
    </tr>
</table>

<form name="form1">
<pgs:listExtensionPools  stylesheet="listextpools.xslt"
                outputxml="false"
                organizationid='<%=orgid%>'/>

</form>
<%
    if( request.getParameter("deletepool").equals("true") ){
%>
    <pgs:deleteExtensionPool
        id='<%=request.getParameter("extensionpoolid")%>' />

<%
    response.sendRedirect("list_all_extpools.jsp?deletepool=false");
    }
%>

</body>
</html>
