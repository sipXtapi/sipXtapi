<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <link rel="stylesheet" type="text/css" href="style/dms_sipxchange.css">
        <title>Call Handling</title>
        <script src="../script/jsVisibility.js"></script>
        <script language="Javascript">
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
                    formSubmit();
                }
            }
        }
        function formSubmit()
        {
			saveFlag = false;
            var form = document.update;
			var formElementsArray = form.elements;
			for( var i = 0; i < formElementsArray.length; i++ )
			{
				if( formElementsArray[i].type == "select-one" )
				{
					var selectedIndex = formElementsArray[i].selectedIndex;
                    var selectedOptionValue = null;
                    if ( selectedIndex != -1 && formElementsArray[i].options[selectedIndex].value == "blank")
                    {
                       formElementsArray[i].options[selectedIndex].value = "";
                    }
                }
			}
			form.submit();
        }


        </script>
    </head>
    <body class="bglight" onLoad="checkVis(); parent.checkMfg()" onUnload="checkSaveFlag()">
    <div id="msg1" style="display: none" class="readonly">The settings on this page apply only to your Pingtel phone(s).</div>
        <form action="../xml/update_configuration_set.jsp" method="post" name="update">
            <table width="560" border="0">
                <tr>
                    <td width="10"><img src="images/spacer.gif" width="10"></td>
                    <td width="100%">
                        <table width="100%" border="0">
                            <tr>
                                <td class="formtitle" height="30">&nbsp;Manage Call Handling</td>
                                <td height="30" align="right"><a class="greentext" href="javascript:void 0" onClick="top.displayHelpPage('help/WebHelp/sipxchange.htm#callhandling.html');">Help</a></td>
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
                                        refpropertygroupid="Call Handling"
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
                                    <hr class="dms" width="560" align="left">
                                </td>
                            </tr>
                            <tr>
                                <td colspan="2" align="center">
                                    <input type="button" value="Save" onClick="formSubmit()"/>
                                </td>
                            </tr>
                        </table>
                    </td>
                </tr>
            </table>
            <input type="hidden" name="ppuser" value="2">
            <input type="hidden" name="etype" value="user">
            <input type="hidden" name="eid" value='<%=request.getParameter("userid")%>'>
            <input type="hidden" name="x_entity_type" value="user">
            <input type="hidden" name="x_entity_id" value='<%=request.getParameter("userid")%>'>
            <input type="hidden" name="redir" value="../sipxchange/callhandling.jsp?userid=<%=request.getParameter("userid")%>&stylesheet=uucallhandling.xslt&mfgid=pingtel&devicetypeid=common&usertype=user">
        </form>
    </body>
</html>
