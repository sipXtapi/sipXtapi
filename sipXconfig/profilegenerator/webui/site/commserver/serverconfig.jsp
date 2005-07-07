<%@ page language="java" contentType="text/html" %>
<%@ page errorPage="error/error.jsp" %>
<%@ taglib uri="xtags" prefix="xslxtags" %>
<%@ page import="com.pingtel.commserver.utility.ServerConfigUIHelper" %>
<%@ page import="com.pingtel.commserver.utility.ErrorMessageBuilder" %>
<%@ page import="java.io.*" %>
<%@ page import="java.util.*" %>
<%@ page import="java.net.URLEncoder" %>
<%
    String servername = request.getParameter("servername");
    ServerConfigUIHelper helper = new ServerConfigUIHelper( servername);
    String generatedxml = helper.generateXML();
    
    // Error / Info text that need to be displayed.
    String message = "" ;
    boolean isError = false;
    if( request.getParameter("message") != null )
    {
        // There was an error. Error message sent in the request
        message = request.getParameter("message") ;
        isError = true;
    }
    else if( request.getParameter("saved") != null )
    {
        // Configuration saved successfully.
        message = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.CONFIGURATION_SAVED );    
    }
    else if( request.getParameter("restored") != null )
    {
        // Configuration saved successfully.
        message = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.CONFIGURATION_RESTORED_TO_DEFAULT );    
    }
     
    if( message.length() > 0 )
    {   
        // Format the message
        boolean criticalError = false;
        message = ErrorMessageBuilder.formatErrorMessage( message, criticalError, isError);
        
        // Insert into the generated xml just before the end.
        int index = generatedxml.indexOf("</details>");
        String firstpart = generatedxml.substring(0, index);
        String secondpart = generatedxml.substring(index);
        generatedxml = firstpart + message + secondpart ;
    }
%>
<html>
    <head>
        <meta http-equiv="Pragma" content="no-cache">
        <meta http-equiv="expires" content="-1">
        <title>Configuration for <%= servername %></title>
        <link rel="stylesheet" href="../style/dms.css" type="text/css">
        <script src="script/jsFunctions.js"></script>
        <%
            out.println( "<script language=\"JavaScript\">");
            out.println( "function populateFormFields() { ");
                if( request.getParameter("message") != null ) 
                {
                    // There was an error saving the message. 
                    // Populate the form fields with the values entered by the user.
                    
                    // 1. Retrieve the values entered by the user from session.
                    TreeMap userEnteredValuesMap = (TreeMap) session.getAttribute( servername ) ;
                    
                    // 2. Generate the javascript for populating the form
                    if( userEnteredValuesMap != null )
                    {
                        while ( userEnteredValuesMap.size() > 0 ) {
                            try {
                                String key = (String) userEnteredValuesMap.firstKey();
                                String value = (String) userEnteredValuesMap.remove( userEnteredValuesMap.firstKey() );
                                if( key.equalsIgnoreCase("additionalsettings") )
                                {
                                    value = URLEncoder.encode(value);
                                    value = value.replace('+', ' ');
                                    out.println("var additional=unescape('" + value + "');");
                                    out.println( "document.forms[0]." + key + ".value=additional;");
                                 }
                                 else
                                 {
                                    out.println( "document.forms[0]." + key + ".value='" + value + "';");
                                 }
                            } catch ( Exception e ) {}
                        }
                    }
                    else
                    {
                        System.out.println("unable to read from session");
                    }
                }
            out.println( "}");
            out.println("</SCRIPT>");
       %>
   </head>
   <body class="bglight" onLoad="MM_goToURL('top.frames[\'command\']','command/serverconfig_cmd.jsp?servername=<%=servername%>');populateFormFields();" onunload="checkSaveFlag('serverconfig')">
        <xslxtags:style xsl="xslt/serverconfig.xslt">
            <xslxtags:param name="servername" value="<%= servername %>"/>
            <%= generatedxml %>
        </xslxtags:style>
   </body>
</html>
