<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String userID = request.getParameter("userid");
%>
<html>
    <head>
        <title>Applications</title>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
        <link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
        <script language="JavaScript">
		function formSubmit( form )
            {

                if ( form.elements[0].selectedIndex == -1 ||
                        form.elements[0].selectedIndex == 0 )
                {}
                else
                {
                    form.submit();
                }
            }
        </script>
    </head>
    <body class="bglight">
        <table width="600" border="0">
            <tr>
                <td><img src="images/spacer.gif" width=5></td>
                <td>
                    <table width="600" border="0">
                        <tr>
                            <td class="formtitle" height="30">Manage Applications</td>
                            <td height="30" align="right"><a class="greentext" href="javascript:void 0" onclick="top.displayHelpPage('help/WebHelp/sipxchange.htm#applications.html');">Help</a></td>
                        </tr>
                        <tr>
                            <td class="errortext_light" colspan="2">
                                <hr class="dms">
                            </td>
                        </tr>
                        <tr>
                            <td class="list" colspan="2">&nbsp;</td>
                        </tr>
                        <tr>
                            <td class="list" colspan="2">
                                <table class="bglist" cellspacing="1" cellpadding="6" border="0" width="100%">
                                    <tr>
                                        <th class="tableheader">Manage Applications</th>
                                    </tr>
                                    <tr>
                                        <td class="tablecontents">
                                            <form method="post" action="assign_app.jsp">
                                                <p><b>Install New Application</b> </p>
                                                <p>Select an application from the list below and click Install.</p>
                                                <p>Application:
                                                    <pgs:listUsersAvailApplications
                                                        stylesheet="assignapplications.xslt"
                                                        userid='<%=request.getParameter( "userid" )%>'
                                                        outputxml="false" />
                                                    <input type="hidden" name="userid" value='<%=request.getParameter("userid")%>'>
                                                    <input type="button" name="cmdSubmitForm" value="Install" onclick="formSubmit(this.form)">
                                                </p>
                                            </form>
                                        </td>
                                    <tr>
                                        <td class="tablecontents">
                                            <form mehtod="post" action="unassign_app.jsp">
                                                <b>Uninstall Existing Applications</b>
                                                <p>Select an application from the list below and click Uninstall. </p>
                                                <p>Application:
                                                    <pgs:listUsersApplications userid='<%=request.getParameter( "userid" )%>'
                                                        stylesheet="unassignapplications.xslt"
                                                        outputxml="false"/>
                                                    <input type="hidden" name="userid" value='<%=request.getParameter("userid")%>'>
                                                    <input type="button" name="cmdSubmitForm2" value="Uninstall" onclick="formSubmit(this.form)">
                                                </p>
                                            </form>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td class="tablecontents">
                                            <p><b>Currently Loaded Applications</b></p>
                                            <pgs:listUsersApplications userid='<%=request.getParameter( "userid" )%>'
                                                stylesheet="listinstalledapplications.xslt"
                                                outputxml="false"/>
                                        </td>
                                    </tr>
                                    <!--<tr>
                                        <td class="tablecontents">
                                            <p><b>Currently Loaded Application Sets</b></p>
                                            <pgs:listUsersApplicationSets userid='<%=request.getParameter( "userid" )%>'
                                                stylesheet="listinstalledappsets.xslt"
                                                outputxml="false"/>
                                        </td>
                                    </tr>-->
                                </table>
                            </td>
                        </tr>
                    </table>
                </td>
            </tr>
        </table>
        <form action="../profile/project_userapps.jsp" method="post" name="frmSendProfile">
            <table width="560" border="0">
                <tr>
                    <td align="center">
                        <input type="submit" value="Send Application Profile">
                        <p class="formtext">* To send your Applications to the phone, please click on the "Send Application Profile"</p>
                        <input type="hidden" name="entityid"
                            value='<%=request.getParameter("userid")%>'>
                    </td>
                </tr>
            </table>
        </form>
   </body>
</html>
