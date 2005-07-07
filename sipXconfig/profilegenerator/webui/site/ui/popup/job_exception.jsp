<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<html>
    <head>
        <title>Job Exception</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
        <script language="JavaScript">
            function closeWindow()
            {
                self.opener.parent.mainFrame.location.reload();
                this.window.close();
            }
        </script>
    </head>
    <body class="bglight">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Job Exception Information</p>
        <pgs:getJobExceptionMessage
            jobid='<%=request.getParameter("jobid")%>'
            stylesheet="jobexception.xslt"
            outputxml="false"/>
        <form>
            <div align="center">
                <input type="button" name="cmdClose" value="Close" onclick="closeWindow()"/>
            </div>
        </form>
    </body>
</html>
