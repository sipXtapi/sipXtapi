<%@ page language="java" contentType="text/html" %>
<%@ page errorPage="error/error.jsp" %>
<%@ taglib uri="xtags" prefix="xslxtags" %>
<%@ page import="com.pingtel.commserver.utility.GlobalConfigUIHelper" %>
<%@ page import="java.util.*" %>
<%
    String redirectUrl = "/pds/commserver/globaldefinitions.jsp";
    GlobalConfigUIHelper helper = new GlobalConfigUIHelper();
	
	if( request.getParameter("restore") != null )
	{
        // Restore to defaults
        String message = helper.restoreToDefaults();
		if( message !=null && message.length() > 0 )
	        redirectUrl += "?message=" + message;
        else
            redirectUrl += "?restored=true" ;
	}
	else
	{
		// Save the changes made by the user
        
        // Read the list of definition names from the session.
        // We need this parameter because:
        // 1. Order in which the definitions are listed are lost when reading from the request
        // 2. Cosmetically, it would be nice to maintain the order so that related definitions
        // are listed one after another.
        String definitionList = (String) session.getAttribute("definitionlist");
        if( definitionList == null )
            definitionList = "";
            
        // Hashtable containing the definition names and their corresponding values
        // as entered by the user.    
	    Hashtable userEnteredValuesHash = new Hashtable();
        
        // Read the definition name - value pairs from the request        
	    Enumeration paramNames = request.getParameterNames();
	    while( paramNames.hasMoreElements() )
	    {
	        String key = (String) paramNames.nextElement();
	        key = key.trim();

            // Check if this is a new parameter added by the user.
            String tempKey = key + ";" ;
            if( definitionList.indexOf(tempKey) == -1 )
                definitionList += key + ";" ;
            else if( definitionList.indexOf(tempKey) > 0 )
            {
                tempKey = ";" + key + ";" ;
                if( definitionList.indexOf(tempKey) == -1 )
                    definitionList += key + ";" ;  
            }  
                        
	        // Get the values associated with this key.
            String[] paramValues = request.getParameterValues( key );
            String value = "";
            if( paramValues.length > 0 )
                value = paramValues[0];
            
            userEnteredValuesHash.put(key,value);
	    }
        userEnteredValuesHash.put("definitionlist", definitionList );
	    
	    if( userEnteredValuesHash.size() > 0 )
	    {
            // Put the Hashtable in the session
           session.setAttribute( "globaldefs", userEnteredValuesHash );
           
	        String message = helper.writeToConfigFile( new Hashtable(userEnteredValuesHash) );
	        if( message !=null && message.length() > 0 )
	            redirectUrl += "?message=" + message;
            else
                redirectUrl += "?saved=true" ;
	    }
	    else
	        redirectUrl += "?message=Failed to receive data from the UI";
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
