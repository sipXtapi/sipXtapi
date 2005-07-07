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

import com.pingtel.commserver.utility.PathLocatorUtility;

/**
 * A basic class for logging information. 
 * Right now, it just writes to standard out.
 * This can be extended to write to approriate log files, etc.
 * 
 * @author Harippriya Sivapatham
 */
public class ErrorMessageBuilder {
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Error messages constants
    public static final int CONFIGURATION_SAVED                = 0 ;
    public static final int CONFIGURATION_RESTORED_TO_DEFAULT  = 1 ;
    public static final int UNKNOWN_SERVER_NAME                = 2 ;
    public static final int CONFIG_FILE_NOT_FOUND              = 3 ;
    public static final int CONFIG_FILE_CORRUPTED              = 4 ;
    public static final int DESCRIPTOR_FILE_NOT_FOUND          = 5 ; 
    public static final int UNABLE_TO_WRITE_TO_CONFIG_FILE     = 6 ;
    public static final int NO_DATA_FROM_REQUEST               = 7 ;
    public static final int DEFAULT_CONFIG_FILE_NOT_FOUND      = 8 ;
    public static final int INVALID_VALUE_MSG_PREFIX           = 9 ;
    public static final int CONFIG_ALREADY_EXISTS              = 10;
    public static final int GLOBAL_CONFIG_FILE_CORRUPTED       = 11;
    public static final int GLOBAL_CONFIGURATION_RESTORED_TO_DEFAULT = 12;
    public static final int GLOBAL_CONFIGURATION_SAVED         = 13;
    public static final int CYCLIC_DEFINITION_REFERENCE        = 14;
    public static final int INVALID_DEFINITION_VALUE           = 15;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    public ErrorMessageBuilder()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public static String getErrorMessage( int errorCode )
    {
        String errorMessage = "";
        switch (errorCode) {
        case UNKNOWN_SERVER_NAME: 
            errorMessage =  "Error: Unknown server name" ;
            break;
        case CONFIG_FILE_NOT_FOUND:
            errorMessage = "Error: Failed to find the associated configuration file.";
            break;
        case CONFIG_FILE_CORRUPTED:
            errorMessage = "Error: Config file corrupted. Please replace this file with the corresponding defaults file." ;
            break;
        case DESCRIPTOR_FILE_NOT_FOUND:
            errorMessage = "Warning: Failed to find the property descriptor XML file.";
            break;
        case UNABLE_TO_WRITE_TO_CONFIG_FILE:
            errorMessage = "Unable to open the config file for writing";
            break;
        case NO_DATA_FROM_REQUEST:
            errorMessage = "Failed to save the changes. Did not receive any data for saving." ;
            break;
        case DEFAULT_CONFIG_FILE_NOT_FOUND:
            errorMessage = "Restore to defaults failed. Unable to find the default config file.";
            break;
        case CONFIGURATION_SAVED:
             errorMessage = "Configuration saved successfully. Changes will take effect on next service restart." ;
             break;
        case INVALID_VALUE_MSG_PREFIX:
            errorMessage = "Error: The following references to global definition settings are invalid. Please verify your entry, or edit your global definitions.<br/>" ;
            break;
        case CONFIGURATION_RESTORED_TO_DEFAULT:
            errorMessage = "Configuration successfully restored to defaults. Changes will take effect on next service restart.";
            break;
        case CONFIG_ALREADY_EXISTS:
            errorMessage = "Error: Adding additional settings failed. Following settings already exist:<br/>" ;
            break;
        case GLOBAL_CONFIG_FILE_CORRUPTED:
            errorMessage = "Failed to parse the global definitions file";
            break;
        case GLOBAL_CONFIGURATION_RESTORED_TO_DEFAULT:
            errorMessage = "Configuration restored to defaults. Changes will take effect on next service restart.";
            break;
        case GLOBAL_CONFIGURATION_SAVED:
            errorMessage = "Configuration saved successfully. Changes will take effect on next service restart.";
            break;
        case CYCLIC_DEFINITION_REFERENCE:
            errorMessage = "Error: Cyclic reference detected between the following settings.<br/>Please verify your entries.<br/>" ;
            break;
        case INVALID_DEFINITION_VALUE:
            errorMessage = "Error: The value entered for the following setting(s) is invalid. Please verify your entries.<br/>" ;
            break;
        }

        return errorMessage;
    }

    /** Constructs the XML snippet used for displaying the error messages.
     *  
     *  @param  errorText   Error message to be displayed
     *  @param  critical    Flag indicating if the error is a critical error.
     *                      For example, when trying to display the global definitions UI,
     *                      if the config.defs file is missing, then it is a critical error.
     *                      Just display the error and no form beneath it.
     *  @param  isError     Flag to indicate if it is just an message and not an error.
     *                      If false (in the case of sucess messages), the text will be in blue.
     *                      If true (in case of invalid value for a definition), the text will be in red.
     *                      It is an error, but not a critical one and hence the form will be displayed.
     * 
     *  @return String      XML snippet
     */ 
    public static String formatErrorMessage( String errorText, 
                                             boolean critical, 
                                             boolean isError )
    {
        String errorMessage = "<error" ;
        if ( critical )
            errorMessage += " critical=\"yes\"" ;

        if( isError )
            errorMessage += " iserror=\"yes\"" ;

        errorMessage += "><![CDATA[\n" + errorText + "]]>\n</error>\n";

        return errorMessage;

    }

    /** Quick fix method. */
    public static String getFileNotFoundError ( String servername, int pathType )
    {
        String errorMessage = "";
        if( pathType == PathLocatorUtility.COMMSERVER_CONFIG_FOLDER )
        {
            String filename = PathLocatorUtility.getConfigFileName( servername );
            if( filename != null && filename != "-1" )
            {
                String path = PathLocatorUtility.getPath( pathType );
                errorMessage = "Error: Configuration file " + path + filename + " not found. " +
                    "Please create it and copy the contents of '" + filename + ".default' file in the same directory." ;
            }
            else
            {
                errorMessage = "Error: Unrecognized server name '" + servername + "'" ;
            }
        }
        else if( pathType == PathLocatorUtility.XML_FOLDER )
        {
            String filename = PathLocatorUtility.getConfigDefinitionFileName( servername );
            if( filename != null && filename != "-1" )
            {
                String path = PathLocatorUtility.getPath( pathType );
                errorMessage = "Warning: File " + path + filename + " not found. Property definition data will not display." ;
            }
            else
            {
                errorMessage = "Error: Unrecognized server name '" + servername + "'" ;
            }
        }
        return errorMessage ;
    }
}

