
<%@ page import="org.jdom.Document, org.jdom.Element, java.io.File,
                 org.jdom.JDOMException,
                 com.pingtel.pds.common.*,
                 com.pingtel.pds.pgs.common.*"
                 errorPage="/ui/error/error.jsp"
%>


<%
    // set introductory text and value to be used for stereotype
    String introText = null;
    String stereotype = null;

    String installStereotype = ConfigFileManager.getInstance().getProperty(
            PathLocatorUtil.getInstance().getPath(
                PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS)+
                "pgs.props",
            "installStereotype");

    // default value handling.
    if(installStereotype == null){
        installStereotype = PDSDefinitions.ENTERPRISE_ST;
    }

    if(installStereotype.equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)){
        introText = "Enterprise Edition Installation";
        stereotype =  String.valueOf(PDSDefinitions.ORG_ENTERPRISE);
    }
    // service provider to be added in the future...
%>


<html>
<head>
   <title>Configuration Server</title>
   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
   <link rel="stylesheet" href="../../style/dms.css" type="text/css">
   <script language="JavaScript">
   var today;
   function myDate(){
       today = new Date();
       return today.toLocaleString();
   }
   function validate(formToCheck){
       var formIsValid = true;
       if ( formToCheck.password.value != formToCheck.repassword.value )
       {
           formIsValid = false;
           alert( "Your password fields do not match.\n" +
                  "Please try again." );
           formToCheck.password.value = "";
           formToCheck.repassword.value = "";
       }

       return formIsValid;
   }

   </script>
</head>

   <body class="bgdark" onLoad="install.orgname.focus()">
<div id="Layer2" style="position:absolute; left:2px; top:1px; width:1312px; height:72px; z-index:2"><img src="../../image/logo.gif" width="305" height="71"><img src="../../image/pingtelbar.gif" width="1000" height="71" border="0"></div>
<div id="Layer3" style="position:absolute; left:0px; top:78px; width:378px; height:26px; z-index:3">
<h1 class="dms">Configuration Server </h1></div>
<div id="Layer4" style="position:absolute; left:479px; top:78px; width:283px; height:29px; z-index:4" align="right" class="date">
  <script type="text/javascript">
               document.write(myDate())
               </script>
</div>
<div id="Layer5" style="position:absolute; left:3px; top:123px; width:695px; height:28px; z-index:5">
<h2 CLASS="dms"><%=introText%></h2></div>
<br><br><br><br><br><br><br><br><br>
   <form action="installorg.jsp" method="post" name="install" onsubmit= 'return validate(this);'>
      <table width="400" border="0" cellspacing="3" cellpadding="6" align="center" class="bglight" >
         <tr>
            <td>&nbsp;</td>
            <td class="formtitle">
               <div align="center">
                  Install Organization
               </div>
            </td>
            <td>&nbsp;</td>
         </tr>
         <tr>
            <td colspan="3">
               <hr>
            </td>
         </tr>
      </table>
      <table width="400" align="center" border="0" class="bglight">
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  Organization Name
               </div>
            </td>
            <td colspan="2">
               <input type="text" name="orgname">
            </td>
         </tr>
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  DNS Domain
               </div>
            </td>
            <td colspan="2">
               <input type="text" name="dnsdomain">
            </td>
         </tr>
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  PIN
               </div>
            </td>
            <td colspan="2">
               <input type="password" name="password">
            </td>
         </tr>
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  Confirm PIN
               </div>
            </td>
            <td colspan="2">
               <input type="password" name="repassword">
            </td>
         </tr>
         <tr>
            <td colspan="2" height="60">&nbsp;</td>

      <td height="60"> <span class="formtitle">*</span>
        <input type="submit" name="Submit" value="Submit">
            </td>
            <td height="60">&nbsp;</td>
         </tr>
      </table>
      <input type="hidden" name="stereotype" value="<%=stereotype%>">
   </form>


<table width="352" border="0" cellpadding="0" cellspacing="0" align="center">
  <tr>
    <td width="17"></td>
    <td width="308"></td>
    <td width="27"></td>
  </tr>
  <tr>
    <td></td>
    <td valign="top">  <p class="notetext">* After submission of this form, you will be taken to a
    log in screen for the organization you have just created. Please log in as
    &quot;User&quot; superadmin, along with the new PIN created above.</p>
</td>
    <td></td>
  </tr>
  <tr>
    <td></td>
    <td></td>
    <td></td>
  </tr>
</table>
</body>
</html>

