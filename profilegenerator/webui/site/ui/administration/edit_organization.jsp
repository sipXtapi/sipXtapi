<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<jsp:useBean id="editOrg" class="com.pingtel.pds.pgs.beans.EditOrganizationBean" />
<%
	boolean orgUpdated = false;
    if("Save".equals(request.getParameter("action"))) {
%>
        <jsp:setProperty name="editOrg" property="*"/>
<%
	    editOrg.submit();
	    orgUpdated = true;
    }    
%>    


<html>
    <head>
<script language="JavaScript">
<!--
function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments;
  document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2)
   eval(args[i]+".location='"+args[i+1]+"'");
}
//-->
</script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css"/>
    <title>EditOrganization</title>
    </head>
    <body class="bglight"
          onLoad="MM_goToURL('parent.frames[\'command\']','../command/command_blank.html');return document.MM_returnValue"
        >
		<table width="600">
		    <tr>
		        <td width="50%" align="left">
		            <h1 class="list">Organization settings</h1>
		        </td>
		        <td width="50%" align="right">
                    <!-- help goes here -->
		        </td>
		    </tr>
		    <tr>
		        <td colspan="2"><hr class="dms"></td>
		    </tr>
		</table>
		<% if( orgUpdated ) { %>
	        <p class="msgtext">Organization settings successfully updated.<br/>
				Go to <a href="list_all_services.jsp" title="Operate Services">Operations</a> page and restart all servers.
			</p>
    	    <pgs:generateCredentials/>
        	<pgs:generatePermissions/>
	        <pgs:generateExtensions/>
    	    <pgs:generateAliases/>
			<pgs:generateAuthExceptions changepointid='*' changepointtype="user"/>
        <% } %>
        <form action="edit_organization.jsp" method="POST">
            <table width="600" class="bglist" cellspacing="1" cellpadding="4" border="0">
                <tr>
                    <th align="left" valign="bottom">Attribute</th>
                    <th>Value</th>
                </tr>                
                <tr class="formtext" nowrap>
                    <td><label for="name">Name:</label></td>
                    <td colspan="2"><input type="text" name="name" id="name" value="<%= editOrg.getName() %>" /></td>
                </tr>
                <tr class="formtext" nowrap>
                    <td><label for="dnsDomain">DNS Domain:</label></td>
                    <td colspan="2"><input type="text" name="dnsDomain" id="dnsDomain" value="<%= editOrg.getDnsDomain() %>" /></td>
                </tr>
            </table>
            <input type="submit" name="action" value="Save">
            <input type="submit" name="action" value="Reload">
        </form>
    </body>
</html>
