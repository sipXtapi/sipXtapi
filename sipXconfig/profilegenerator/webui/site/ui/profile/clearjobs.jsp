<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<html>
<head>
<script language="javascript">
function reloadMain()
{
    parent.mainFrame.location.reload();
    location.replace("../command/list_jobs_cmd.html");
}
</script>
</head>
<body onload="reloadMain()">
<pgs:clearFinishedJobs/>
</body>
</html>
