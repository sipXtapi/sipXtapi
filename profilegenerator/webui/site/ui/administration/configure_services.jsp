<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
<head>
    <title>ConfigureServices</title>
    <link rel="stylesheet" type="text/css" href="../../style/dms.css">
    <script language="JavaScript">
    <!--
    function MM_goToURL() { //v3.0
        var i, args=MM_goToURL.arguments;
        document.MM_returnValue = false;
        for (i=0; i<(args.length-1); i+=2)
            eval(args[i]+".location='"+args[i+1]+"'");
    }
    function MM_swapImgRestore() { //v3.0
        var i,x,a=document.MM_sr; for (i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
    }
    
    function MM_preloadImages() { //v3.0
        var d=document; if (d.images) {
            if (!d.MM_p) d.MM_p=new Array();
            var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for (i=0; i<a.length; i++)
                if (a[i].indexOf("#")!=0) {
                    d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];
                }
        }
    }
    
    function MM_findObj(n, d) { //v4.01
        var p,i,x;  if (!d) d=document;if ((p=n.indexOf("?"))>0&&parent.frames.length) {
            d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);
        }
        if (!(x=d[n])&&d.all) x=d.all[n];for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
        for (i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
        if (!x && d.getElementById) x=d.getElementById(n);return x;
    }
    
    function MM_swapImage() { //v3.0
        var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for (i=0;i<(a.length-2);i+=3)
            if ((x=MM_findObj(a[i]))!=null) {
                document.MM_sr[j++]=x; if (!x.oSrc) x.oSrc=x.src;x.src=a[i+2];
            }
    }
    
    function getFrameLocation(){
        return frame.location;
    }
    
    //-->
    </script>
</head>
    <body class="bglight"
    onLoad="MM_goToURL('parent.frames[\'command\']','../command/configure_services_cmd.html');
    MM_preloadImages('../buttons/sm_btn_f2.gif');return document.MM_returnValue">

    <table width="600">
        <tr>
            <td width="50%" align="left"><h1 class="list">Configuration</h1></td>
            <td width="50%" align="right"><a href="#" onclick="window.parent.MM_openBrWindow('/pds/commserver/help/commserver/WebHelp/configure_services.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')" class="formtext">Help</a></td>
        </tr>
        <tr>
            <td colspan="2">
                <hr class="dms">
            </td>
        </tr>
    </table>
    <form>
    <table cellspacing="1" cellpadding="4" border="0" width="600" class="bglist">
        <tr>
            <th width="450">Name</th>
            <th>Configure</th>
        </tr>
        <tr>
            <td width="450">Global Definitions</td>
            <td align="center"><a onFocus="if(this.blur)this.blur()" onMouseOver="MM_swapImage('sm_btn_delstart','','../buttons/sm_btn_f2.gif',1)" onMouseOut="MM_swapImgRestore()" href="/pds/commserver/globaldefinitions.jsp">
                <img border="0" height="17" width="23" src="../buttons/sm_btn.gif" name="sm_btn_delstart"/></a></td>
        </tr>
        <tr>
            <td>CommServer</td>
            <td align="center"><img border="0" height="17" width="23" src="../../image/spacer.gif"/></td>
        </tr>

        <tr>
            <td><div style="text-indent: 2em">SipAuthProxy</div></td>
            <td align="center"><a onFocus="if(this.blur)this.blur()" onMouseOver="MM_swapImage('sm_btn_delstart','','../buttons/sm_btn_f2.gif',1)" onMouseOut="MM_swapImgRestore()" href="/pds/commserver/serverconfig.jsp?servername=authproxy">
                <img border="0" height="17" width="23" src="../buttons/sm_btn.gif" name="sm_btn_delstart"/></a></td>
        </tr>
        <tr>
            <td><div style="text-indent: 2em">SipProxy</div></td>
            <td align="center"><a onFocus="if(this.blur)this.blur()" onMouseOver="MM_swapImage('sm_btn_delstart','','../buttons/sm_btn_f2.gif',1)" onMouseOut="MM_swapImgRestore()" href="/pds/commserver/serverconfig.jsp?servername=proxy">
                <img border="0" height="17" width="23" src="../buttons/sm_btn.gif" name="sm_btn_delstart"/></a></td>
        </tr>
        <tr>
            <td><div style="text-indent: 2em">SipRegistrar</div></td>
            <td align="center"><a onFocus="if(this.blur)this.blur()" onMouseOver="MM_swapImage('sm_btn_delstart','','../buttons/sm_btn_f2.gif',1)" onMouseOut="MM_swapImgRestore()" href="/pds/commserver/serverconfig.jsp?servername=registrar">
                <img border="0" height="17" width="23" src="../buttons/sm_btn.gif" name="sm_btn_delstart"/></a></td>
        </tr>
        <tr>
            <td><div style="text-indent: 2em">SipStatus</div></td>
            <td align="center"><a onFocus="if(this.blur)this.blur()" onMouseOver="MM_swapImage('sm_btn_delstart','','../buttons/sm_btn_f2.gif',1)" onMouseOut="MM_swapImgRestore()" href="/pds/commserver/serverconfig.jsp?servername=statusserver">
                <img border="0" height="17" width="23" src="../buttons/sm_btn.gif" name="sm_btn_delstart"/></a></td>
        </tr>
    </table>
    </form>
    <br/>
    <br/>
</body>
</html>
