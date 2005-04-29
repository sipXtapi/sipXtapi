<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
    <title>Delete Extensions</title>
    <script src="../script/validate.js">
    </script>
    <script language="JavaScript">
        function formSubmit( form )
        {
            var minValue = form.minextension.value;
            var maxValue = form.maxextension.value;
            var extPat = /^[0-9]+$/;
            var minTest = minValue.match( extPat );
            var maxTest = maxValue.match( extPat );

            if( minTest == null ) {
                alert ( "The Min Extension must be a positive integer, please try again." );
                form.minextension.value = "";
            }
            else if( maxTest == null ) {
                alert ( "The Max Extension must be a positive integer, please try again." );
                form.maxextension.value = "";
            }
            else if( parseInt( minTest ) > parseInt( maxTest ) ) {
                alert( "The Max Extension field must be >= \n" +
                    "the Min Extension field. " );
                form.maxextension.value = "";
            }
            else {
                form.submit_flag.value = "true";
                form.submit();
            }
        }
        function closeWindow()
        {
            this.window.close();
        }

    </script>
    <link rel="stylesheet" href="../../style/dms.css" type="text/css">
</head>
    <body class="bglight">
    <p class="formtitle" align="center">
    Delete Extensions
    </p>
    <form name="frmAddExtPool" method="get" action="del_extensions.jsp">
        <div id="form">

            <table width="150" border="0" cellspacing="3" cellpadding="0" align="center">
                <tr>
                    <td class="formtext" nowrap>Min Extension</td>
                    <td>
                        <input type="text" name="minextension" size="14">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap>Max Extension</td>
                    <td>
                        <input type="text" name="maxextension" size="14">
                    </td>
                </tr>
                <tr>
                    <td colspan="4" align="center">
                        <input type="button" name="cmdSubmitForm" value="Submit" onclick="formSubmit(this.form)">
                        &nbsp;&nbsp;
                        <input type="button" name="cmdCloseWindow" value="Close" onclick="closeWindow()"/>
                    </td>
                </tr>
            </table>
        </div>
        <input type="hidden" name="submit_flag" value="false">
        <input type="hidden" name="extensionpoolid"
            value='<%=request.getParameter("extensionpoolid")%>' >
    </form>
    <pgs:extensionPoolDetails
        extensionpoolid= '<%=request.getParameter("extensionpoolid")%>'
        stylesheet="currentextensions.xslt"
        outputxml="false" />
<%
    if ( request.getParameter("submit_flag").equals("true")) {
%>
     <pgs:deleteExtensionRange
        minextension='<%=request.getParameter("minextension")%>'
        maxextension='<%=request.getParameter("maxextension")%>'
        extensionpoolid='<%=request.getParameter("extensionpoolid")%>' />
<%
    response.sendRedirect("del_extensions.jsp?extensionpoolid=" +
            request.getParameter("extensionpoolid") + "&submit_flag=false");
    }

%>
    </body>
</html>
