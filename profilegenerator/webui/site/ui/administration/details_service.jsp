<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%@ page import = "com.pingtel.pds.pgs.sipxchange.process.*"    %>
<%@ page import = "com.pingtel.pds.pgs.sipxchange.*"    %>
<%@ page import = "java.util.*"    %>
<%@ page import = "java.io.*"    %>
<%@ page import =  "org.jdom.*" %>
<%@ page import =  "org.jdom.input.*" %>

<html>
<head>
<title>ListServices</title>
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<script language="JavaScript">
<!--
function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments;
  document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2)
   eval(args[i]+".location='"+args[i+1]+"'");
}
function deleteApplication( id ){
    var msg = "Are you sure you want to delete the application?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        location = "delete_application.jsp?applicationid=" + id;
    }
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
<%
String strCmdPageURL = "../command/details_service_cmd.jsp";
strCmdPageURL += "?process="+java.net.URLEncoder.encode(request.getParameter("process"));
if( request.getParameter("location") != null )
    strCmdPageURL += "&location="+java.net.URLEncoder.encode(request.getParameter("location"));
if( request.getParameter("command") != null )
    strCmdPageURL += "&command="+java.net.URLEncoder.encode(request.getParameter("command"));
if( request.getParameter("redirect") != null )
    strCmdPageURL += "&redirect="+java.net.URLEncoder.encode(request.getParameter("redirect"));
%>
<body class="bglight"
	onLoad="MM_goToURL('parent.frames[\'command\']','<%=strCmdPageURL%>');
	MM_preloadImages('../buttons/sm_btn_f2.gif','buttons/add_appl_btn_f2.gif');return document.MM_returnValue">



<pgs:services  stylesheet="detailsservice.xslt" 
                     outputxml="false" 
                     process= '<%=request.getParameter("process")%>'
                     location= '<%=request.getParameter("location")%>'
                     command= '<%=request.getParameter("command")%>'  
                     redirect='<%=request.getParameter("redirect")%>' />


</body>
</html>
