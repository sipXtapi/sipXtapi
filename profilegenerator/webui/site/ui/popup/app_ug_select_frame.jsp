<%@ page errorPage="/ui/error/error.jsp" %>
<%
    String userGroupID = request.getParameter( "usergroupid" );
%>

<html>
    <head>
        <title></title>
        <link rel="stylesheet" type="text/css" href="../../style/dms.css">
        <script language="JavaScript">
            function loadFrame(src)
            {
                if( src.value == "1" )
                {
                    parent.mainFrame.location = "form_assign_application_toug.jsp?usergroupid=<%=userGroupID%>";
                }
                else
                {
                    parent.mainFrame.location = "form_assign_applicationset_toug.jsp?usergroupid=<%=userGroupID%>";
                }
            }
        </script>
    </head>
    <body class="bglight">
        <div align="center" valign="middle" class="formtext">
            <form>
                <input type="radio" name="frameToLoad" value="1" checked onclick="loadFrame(this)">Applications
                &nbsp;&nbsp;
                <input type="radio" name="frameToLoad" value="2" onclick="loadFrame(this)">Application Sets
            </form>
        </div>
    </body>
</html>