/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.commserver.utility;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.StringTokenizer;
import java.util.TreeMap;

import com.pingtel.pds.common.XMLSupport;


/**
 * This utility provides various functions for constructing the UI for server config files. 
 * 
 * @author Harippriya Sivapatham
 */
public class ServerConfigUIHelper {
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    // All server configurations are defined as key:value
    private static final String KEY_VALUE_SEPARATOR = " :" ;
    private static final String ERROR_KEY           = "error";

    
    // Name of the server with which a instance of this class is associated with.
    private String server;
    private LogUtility logger = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs the helper with the name of the server it is associated with.
     */
    public ServerConfigUIHelper( String servername )
    {
        // Set the name of the server with which this instance is associated with
        server = servername;

        // Initialize the logging module
        logger = new LogUtility();
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Reads the config file and the configuration descriptor file and then 
     * generates the XML for display on the UI.
     * 
     */ 
    public String generateXML()
    {
        // 1. Add the root element - <details>
        StringBuffer xmlContent = new StringBuffer("<details>");

        // 2. Add the configuration set - config and value defined in <server>-config.in file
        xmlContent.append( generateConfigSetXML() );
         
        // 3. Add the contents of the property definition XML
        String configDefinitionXML =  PathLocatorUtility.getConfigDefinitionFileLocation( server );
        if ( configDefinitionXML != null && configDefinitionXML != "-1" ) {
            
            try {
                BufferedReader in = new BufferedReader( new FileReader(configDefinitionXML) );
                String str;
                while ((str = in.readLine()) != null) {
                    xmlContent.append(str);
                } 
            } catch (FileNotFoundException fe) {
                fe.printStackTrace();
                //String errorString = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.DESCRIPTOR_FILE_NOT_FOUND );
                String errorString = ErrorMessageBuilder.getFileNotFoundError( server, PathLocatorUtility.XML_FOLDER );
                xmlContent.append( ErrorMessageBuilder.formatErrorMessage(errorString, false, true) );
                logger.writeToLog( "File not found: " + configDefinitionXML );
            } catch (IOException e) {
                System.out.println( "Exception ");
            } catch (NullPointerException npe )
            {
                System.out.println( "Nullpointer exception");
            }
        }
        else
        {
            // Failed to get the name of the property descriptor XML
            //String errorString = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.DESCRIPTOR_FILE_NOT_FOUND );
            String errorString = ErrorMessageBuilder.getFileNotFoundError( server, PathLocatorUtility.XML_FOLDER );
            xmlContent.append( ErrorMessageBuilder.formatErrorMessage(errorString, false, true) );
            logger.writeToLog( "Failed to get the name of the property descriptor XML for server " + server );
        }
            
        xmlContent.append("</details>");

        return xmlContent.toString();

    }

    public String writeToConfigFile( TreeMap fileContentsMap )
    {
        String errorMessage = "";

        // 1. Retrieve the additional settings received from the request.
        String additionalSettings = (String) fileContentsMap.remove(new String("additionalsettings"));

        if ( additionalSettings.length() > 0 ) {
            // 2. Separate the additional settings into key-value pairs.
            StringTokenizer st = new StringTokenizer( additionalSettings, "\r");
            while (st.hasMoreTokens()) {
                String keyvaluepair = st.nextToken();
                // separate out the key and the value
                int separatorIndex = keyvaluepair.indexOf( ":" );
                if ( separatorIndex != -1 ) {
                    String key = keyvaluepair.substring(0, separatorIndex).trim();
                    String value = "" ;
                    if ( separatorIndex < (keyvaluepair.length()-1) )
                        value = keyvaluepair.substring(separatorIndex+1).trim();

                    // Add the additional settings to the TreeMap
                    if( fileContentsMap.containsKey(key ) )
                    {
                        if( errorMessage.length() == 0 )
                            errorMessage = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.CONFIG_ALREADY_EXISTS ) ;
                        errorMessage += key + "<br/>" ; 
                    }
                    else
                    {
                        fileContentsMap.put(key, value);
                    }
                }
            }
        }

        // 3. Construct the file content ( in the format configname : value )
        if( errorMessage.length() == 0 )
        {
            StringBuffer fileContentsBuffer = new StringBuffer();
            while ( fileContentsMap.size() > 0 ) {
                try {
                    String key = (String) fileContentsMap.firstKey();
                    String value = (String) fileContentsMap.remove( fileContentsMap.firstKey() );
    
                    // Check if the value is valid.
                    if( valueIsValid( value ) )
                        fileContentsBuffer.append( key + " : " + value + "\n");
                    else
                    {
                        // Construct the error message indicating the configuration name with an invalid value
                        if( errorMessage.length() == 0 )
                            errorMessage = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.INVALID_VALUE_MSG_PREFIX ) ;
                        errorMessage += value + "<br/>" ;
                    }
                } catch ( Exception e ) {
                }
            }
    
            if( errorMessage.length() == 0 )
            {
                if ( fileContentsBuffer.length() > 0 ) {
                    // 4. Get the name of the config file.
                    String configfile =  PathLocatorUtility.getConfigFileLocation( server );
                    if ( configfile != null && configfile != "-1" ) {
                        // 5. Write to the config.in file.
                        try {
                            BufferedWriter out = new BufferedWriter(new FileWriter(configfile));
                            out.write(fileContentsBuffer.toString());
                            out.close();
                    
                        } catch ( Exception e ) {
                            //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.UNABLE_TO_WRITE_TO_CONFIG_FILE );
                            errorMessage += "Error: Cannot write to " + XMLSupport.encodeUtf8(configfile) + " file. Please verify that this file has write privileges." ;

                        }
                    } else {
                        errorMessage = ErrorMessageBuilder.getFileNotFoundError ( server, PathLocatorUtility.COMMSERVER_CONFIG_FOLDER );
                        //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.CONFIG_FILE_NOT_FOUND );
                    }
                } else {
                errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.NO_DATA_FROM_REQUEST );
                }
            }
        }
                
        return errorMessage;
    }

    public String restoreToDefaults()
    {
        // Return string
        String errorMessage = "";

        // 1. Get the location of the config file and the default config file
        String configFileLocation       = PathLocatorUtility.getConfigFileLocation( server );
        
        if( configFileLocation != null && configFileLocation != "-1" )
        {
            String defaultConfigFileLocation= PathLocatorUtility.getDefaultConfigFileLocation( configFileLocation );

            // 2. Check if the default config file exists
            File defaultFile = new File( defaultConfigFileLocation );
            if( defaultFile.exists() )
            {
                // 3. Delete the config file and make a copy of the default config file
                File configFile = new File( configFileLocation );
                configFile.delete();

                // Copy the default file
                try
                {
                    FileReader in = new FileReader(defaultFile);
                    FileWriter out = new FileWriter(configFile);
                    int c;
            
                    while ((c = in.read()) != -1)
                       out.write(c);
            
                    in.close();
                    out.close();
                }
                catch (IOException e)
                {
                    // error.
                    //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.UNABLE_TO_WRITE_TO_CONFIG_FILE );
                    errorMessage += "Error: Restore to defaults failed. Cannot write to " + XMLSupport.encodeUtf8(configFileLocation) + " file. Please verify that this file has write privileges." ;
                    logger.writeToLog( "Restore to defaults failed. Unable to write to " + configFileLocation );
                }
                
            }
            else
            {
                // default config does not exist
                //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.DEFAULT_CONFIG_FILE_NOT_FOUND );
                errorMessage = "Error: Restore to defaults failed. File " + XMLSupport.encodeUtf8(defaultConfigFileLocation) + " not found." ;
                logger.writeToLog( "Restore to defaults failed. Default config file does not exist at " + defaultConfigFileLocation );
            }
        }
        else
        {
            // failed to obtain the file locations
        }

                return errorMessage;
    }

    
//////////////////////////////////////////////////////////////////////////////
// Private Methods
////

    private String generateConfigSetXML()
    {
        String errorString = "" ;
        StringBuffer configurationSet = new StringBuffer( "<configurationset>" );

        // 1. Parse the config file
        TreeMap configPropertiesMap = new TreeMap( parseConfigFile() );
        while ( configPropertiesMap.size() > 0 ) {
            try {
                // Read each configuration(key) - value pair
                String key = configPropertiesMap.firstKey().toString().trim();
                String value = configPropertiesMap.remove( configPropertiesMap.firstKey() ).toString().trim();
                if ( !key.equalsIgnoreCase(ERROR_KEY) ) {
                    // Format the configuration name and their value into XML
                    configurationSet.append("<configuration>\n");
                    configurationSet.append("<configname><![CDATA[" + key + "]]></configname>\n" );
                    configurationSet.append("<configvalue><![CDATA[" + value + "]]></configvalue>\n" );
                    configurationSet.append("</configuration>\n" );
                } else {
                    // Critical error in parsing the config file. 
                    errorString = ErrorMessageBuilder.formatErrorMessage(value, true, true);
                }
            } catch ( Exception e ) {
            }
        }

        configurationSet.append("</configurationset>");

        // Append the error string at the end
        // This makes <configurationset> and <error> to be at the same level under root
        if ( errorString.length() > 0 )
            configurationSet.append( errorString );

        return configurationSet.toString();
    }

    private TreeMap parseConfigFile()
    {
        // This will be populated and returned by this method.
        TreeMap configPropertiesMap = new TreeMap();

        // 1. Get the location of the config file.
        String fileLocation =   PathLocatorUtility.getConfigFileLocation( server );

        // 2. Read the file and extract the properties and the value.
        if ( fileLocation != null && fileLocation != "-1") {
            try {
                BufferedReader in = new BufferedReader(new FileReader(fileLocation));
                String str;
                while ((str = in.readLine()) != null) {
                    // Parse each line to separate key (configuration name) and data
                    int separatorIndex = str.indexOf(KEY_VALUE_SEPARATOR);
                    if ( separatorIndex != -1) {
                        String key = str.substring( 0, separatorIndex).trim();
                        String value = "";
                        if ( separatorIndex < (str.length()-2) )
                            value = str.substring(separatorIndex+2).trim();

                        // Add a entry to the hash table.
                        configPropertiesMap.put(key,value);
                    }
                }
                in.close();

                if ( configPropertiesMap.size() == 0 ) {
                    // Failed to obtain name-value pairs
                    //String errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.CONFIG_FILE_CORRUPTED );

                    String errorMessage = "Error: Configuration file " + fileLocation + " is corrupted. " +
                        "Please replace it with the corresponding '.default' file in the same directory." ;
                    configPropertiesMap.put(ERROR_KEY, errorMessage);
                    logger.writeToLog( "File corrupted : " + fileLocation );
                }

            } catch (FileNotFoundException  e) {
                // Failed to read the config file
                //String errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.CONFIG_FILE_NOT_FOUND );
                String errorMessage = ErrorMessageBuilder.getFileNotFoundError ( server, PathLocatorUtility.COMMSERVER_CONFIG_FOLDER );
                configPropertiesMap.put(ERROR_KEY, errorMessage);
                logger.writeToLog( "Failed to read the file " + fileLocation );
            } catch (IOException ie ) {
            }
        } else {
            // Failed to retrieve the location of the config file
            //String errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.UNKNOWN_SERVER_NAME );
            String errorMessage = ErrorMessageBuilder.getFileNotFoundError ( server, PathLocatorUtility.COMMSERVER_CONFIG_FOLDER );
            configPropertiesMap.put(ERROR_KEY, errorMessage);
            logger.writeToLog( "Unknown server name " + server );
        }

        return configPropertiesMap;
    }

    
    private boolean valueIsValid( String value )
    {
        boolean isValid = false ;

        // For now, check only if the value starts with '${'.
        int startIndex = value.indexOf("${");
        if( startIndex != -1 )
        {
            int endIndex = value.indexOf("}");
            if( endIndex != -1 )
            {
                String strValue = value.substring(startIndex+2, endIndex );
                GlobalConfigUIHelper globalConfigHelper = new GlobalConfigUIHelper();
                if( globalConfigHelper.isDefinedInConfigDefs( strValue ) )
                    isValid = true;
            }
        }
        else
        {
            isValid = true ;
        }

        return isValid ;
    }

    public static void main(String[] args)
    {
        ServerConfigUIHelper helper = new ServerConfigUIHelper( "proxy" );
        String xmloutput = helper.generateXML();
    }
}
