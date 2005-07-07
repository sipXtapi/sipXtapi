<%@ page errorPage="/ui/error/error.jsp"
    import="  com.pingtel.pds.common.*,
            javax.naming.*,
            java.util.*,
            com.pingtel.pds.pgs.profile.*,
            com.pingtel.pds.pgs.phone.*,
            com.pingtel.pds.pgs.user.*,
            com.pingtel.pds.pgs.common.*,
            com.pingtel.pds.pgs.entity.EntityDetailsModifier,
            com.pingtel.pds.pgs.entity.EntityDetailsModifierHome,
            javax.rmi.*,
            java.net.URLDecoder,
            javax.servlet.jsp.JspException"%>

<%@ taglib uri="pgstags" prefix="pgs" %>
<%

    // BUG XCF-39: When the following line was at the bottom Tomcat 4.0.1 threw
    // a NullException in Jasper's ClassLoader. Although I could not
    // determine why this was happending, I moved this to the top
    // of the page, before any output is generated, which is the proper place to put it according 
    // to server API. This gaurontees redirect get's into first HTTP header making it
    // effective.
    response.sendRedirect( request.getParameter( "redir" ) );
%>
<html>
<head>
<title>Update Configuration Set</title></head>
<body>
<%!
    EntityDetailsModifierHome edmHome = null;
%>

<%
    InitialContext initial = new InitialContext();
    ArrayList formStrings = new ArrayList();

    try {
        if ( edmHome == null ) {
            edmHome = (EntityDetailsModifierHome) initial.lookup( "EntityDetailsModifier" );
        }
        EntityDetailsModifier modifier = edmHome.create();

        for (   Enumeration jsStrings = request.getParameterNames( );
                jsStrings.hasMoreElements(); ) {

            StringBuffer value = new StringBuffer();
            String paramName = (String) jsStrings.nextElement();
            //String paramValue = URLDecoder.decode( request.getParameter( paramName ) );
            String paramValue = request.getParameter(paramName);

            if ( paramName.equals( "redir" ) ) {
                continue;
            }
            else {
                formStrings.add( paramName + "^^^" + paramValue );
            }
        }

        modifier.updateEntityDetails ( formStrings );
    }
    catch ( Exception ex ) {
        throw new JspException ( ex.toString() );
	
    }
%>
<%
    if ( request.getParameter( "ppuser" ) != null )
    {
%>
    <pgs:projectProfiles    profiletypes = '<%= request.getParameter("ppuser") %>'
                            entitytype= '<%=request.getParameter("etype")%>'
                            entityid= '<%=request.getParameter("eid")%>'/>
<%
    }
%>


</body>
</html>
