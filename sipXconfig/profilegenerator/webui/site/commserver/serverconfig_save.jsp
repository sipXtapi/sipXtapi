<%@ page language="java" contentType="text/html" %>
<%@ page errorPage="error/error.jsp" %>
<%@ taglib uri="xtags" prefix="xslxtags" %>
<%@ page import="com.pingtel.commserver.utility.ServerConfigUIHelper" %>
<%@ page import="java.util.*" %>
<%
	String servername = request.getParameter("servername");
    String redirectUrl = "/pds/commserver/serverconfig.jsp?servername=" + servername;
    ServerConfigUIHelper helper = new ServerConfigUIHelper( servername);
	
	if( request.getParameter("restore") != null )
	{
        // Restore to defaults
		String message = helper.restoreToDefaults();
		if( message !=null && message.length() > 0 )
	        redirectUrl += "&message=" + message;
        else
            redirectUrl += "&restored=true" ;
	}
	else
	{
		// Save the changes made by the user
    
	    // Read the parameters in the request
	    TreeMap userEnteredValuesMap = new TreeMap();
	    Enumeration paramNames = request.getParameterNames();
	    while( paramNames.hasMoreElements() )
	    {
	        // Get a parameter from the request
	        String key = (String) paramNames.nextElement();
	        key.trim();
	        
	        if( !key.equalsIgnoreCase( "servername" ) )
	        {
	            // Get the values associated with this key.
	            String[] paramValues = request.getParameterValues( key );
	            String value = "";
	            if( paramValues.length > 0 )
	                value = paramValues[0];
                
	            userEnteredValuesMap.put(key,value);
	        }
	    }
	    
	    if( userEnteredValuesMap.size() > 0 )
	    {
            // Put the TreeMap in the session

           session.setAttribute( servername, userEnteredValuesMap );
           
	        String message = helper.writeToConfigFile( new TreeMap(userEnteredValuesMap) );
	        if( message !=null && message.length() > 0 )
	            redirectUrl += "&message=" + message;
            else
                redirectUrl += "&saved=true" ;
	    }
	    else
	        redirectUrl += "&message=Failed to receive data from the UI";
	}
%>
                
<html>
    <body class="bglight">
        <SCRIPT LANGUAGE="Javascript">
			if ( document.layers )
			{	
				// This is Netscrape 4.x - it accepts '#' in the URL, but does not like the substitution of '%23' for #.
				// Hence undo any encoding that you did.
				redirectURL = unescape("<%= redirectUrl %>");	
				setTimeout('location.href="' + redirectURL + '"', 0);
			}
			else
			{
				setTimeout('location.href="<%= redirectUrl %>"', 0);
			}
			</SCRIPT>
   </body>
</html>
