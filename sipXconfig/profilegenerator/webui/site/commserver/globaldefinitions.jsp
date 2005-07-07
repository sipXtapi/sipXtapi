<%@ page errorPage="error/error.jsp" %>
<%@ page language="java" contentType="text/html" %>
<%@ taglib uri="xtags" prefix="xslxtags" %>
<%@ page import="com.pingtel.commserver.utility.GlobalConfigUIHelper" %>
<%@ page import="com.pingtel.commserver.utility.ErrorMessageBuilder" %>
<%@ page import="java.io.*" %>
<%@ page import="java.util.*" %>
<%@ page import="java.net.URLEncoder" %>
<%
    // XML representation of the contents of global definitions file.
    // XSLT stylesheet will be applied to this XML to generate the HTML for the UI
    String generatedxml = "";
    
    // Holds error/info messages
    String message = "" ;

    // Helper object with methods for building the UI
    GlobalConfigUIHelper helper = new GlobalConfigUIHelper();
    
    if( request.getParameter("message") != null )
    {
        // User made changes and tried to save. Save failed...
        
        // Contains the cause for save failure.
        message = request.getParameter("message") ;
        
        // Hashtable with the name value pairs entered by the user
        Hashtable userEnteredValuesHash = (Hashtable) session.getAttribute( "globaldefs" );
        
        // Construct the XML based on the values entered by the user.
        generatedxml = helper.generateXML(  new Hashtable(userEnteredValuesHash), message );
        
    }
    else
    {
        // Construct the XML based on the contents of the config.defs file
        generatedxml = helper.generateXML();
        
        if( request.getParameter("saved") != null )
        {
            // Configuration saved successfully.
            message = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.GLOBAL_CONFIGURATION_SAVED );    
        }
        else if( request.getParameter("restored") != null )
        {
            // Configuration saved successfully.
            message = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.GLOBAL_CONFIGURATION_RESTORED_TO_DEFAULT );    
        }
     
        if( message.length() > 0 )
        {   
            // Format the message
            boolean criticalError = false;    
            message = ErrorMessageBuilder.formatErrorMessage( message, criticalError, false);
            
            // Insert into the generated xml just before the end.
            int index = generatedxml.indexOf("</definitions>");
            String firstpart = generatedxml.substring(0, index);
            String secondpart = generatedxml.substring(index);
            generatedxml = firstpart + message + secondpart ;
        }
   }
   
    // find the definitionlist and put it in the session
    String definitionList = helper.getDefinitionList( generatedxml );
    session.setAttribute("definitionlist", definitionList);
%>
<html>
    <head>
        <meta http-equiv="Pragma" content="no-cache">
        <meta http-equiv="expires" content="-1">
        <title>Manage Global Definitions</title>
        <link rel="stylesheet" href="../style/dms.css" type="text/css">
        <script src="script/jsFunctions.js"></script>
  </head>
   <body class="bglight" onLoad="MM_preloadImages('../image/del.gif');MM_goToURL('top.frames[\'command\']','command/globalconfig_cmd.html');populateFormFields();" onunload="checkSaveFlag('globalconfig')">
        <xslxtags:style xsl="xslt/globaldefinitions.xslt">
            <%= generatedxml %>
        </xslxtags:style>
   </body>
</html>
