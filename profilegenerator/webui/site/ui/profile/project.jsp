<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
   String profileTypes = "";
   if ( request.getParameter( "ppdevice" ) != null )
   {
      profileTypes = request.getParameter( "ppdevice" ) + ",";
   }
   if ( request.getParameter( "ppuser" ) != null )
   {
       profileTypes = profileTypes + request.getParameter( "ppuser" ) + ",";
   }
   profileTypes = profileTypes.substring( 0, profileTypes.length() - 1 );
%>



<html>
<head>
<script language="JavaScript">
function closeWindow()
{
    this.window.close();
}
</script><link rel="stylesheet" href="../../style/dms.css" type="text/css">
<title>ProjectProfiles</title>
</head>
<body class="bglight" onload="closeWindow()">
<%-- The orgid parameter is optional, if it is not specified
	 the taglib will see it as null and look in the session object --%>

<pgs:projectProfiles    profiletypes = '<%= profileTypes %>'
                        entitytype= '<%=request.getParameter("entitytype")%>'
                        entityid= '<%=request.getParameter("entityid")%>'/>

</body>
</html>
