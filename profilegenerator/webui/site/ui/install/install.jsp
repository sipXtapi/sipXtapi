<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="/ui/error/error.jsp" %>

<jsp:useBean id="installOrg" class="com.pingtel.pds.pgs.beans.InstallOrganizationBean" />


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
               <input type="text" name="orgName">
            </td>
         </tr>
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  DNS Domain
               </div>
            </td>
            <td colspan="2">
               <input type="text" name="dnsDomain" value="<%= installOrg.getDnsDomain() %>"/>
            </td>
         </tr>
         <tr>
            <td colspan="2" class="formtext">
               <div align="right">
                  Authorization Realm
               </div>
            </td>
            <td colspan="2">
               <input type="text" name="realm" value="<%= installOrg.getRealm() %>"/>
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

      <td height="60">
        <input type="submit" name="Submit" value="Submit">
            </td>
            <td height="60">&nbsp;</td>
         </tr>
      </table>
      <input type="hidden" name="stereotype" value="<%= installOrg.getStereotype() %>"/>
   </form>


<table width="352" border="0" cellpadding="0" cellspacing="0" align="center">
  <tr>
    <td width="17"></td>
    <td width="308"></td>
    <td width="27"></td>
  </tr>
  <tr>
    <td></td>
    <td valign="top">  
<p class="notetext">Fill in the fields on this screen and press the Submit button 
    to access the login screen. At the login screen type <em>superadmin</em> for the username and 
    enter the PIN/password you created on this screen.<br/> After you login you need to 
    <strong>restart your services</strong> before you can use the installed system.</p>
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

