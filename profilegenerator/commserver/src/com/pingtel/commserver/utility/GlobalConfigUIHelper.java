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
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.Vector;

import com.pingtel.pds.common.XMLSupport;

/**
 * Provides various functions for constructing the UI for global definition file (config.defs)
 * TODO: there are couple of things that should be improved here
 * - upgrade to new collection classes (List, Map)
 * - co not use mamber fields as to pass parameters between methods
 * - allow to pass location to read (constructor) and write functions - do not store it in the class
 * @author Harippriya Sivapatham
 */
public class GlobalConfigUIHelper {
    /** names of the confi.defs properties */
    public final static String SIPXCHANGE_DOMAIN_NAME = "SIPXCHANGE_DOMAIN_NAME";
    public final static String SIPXCHANGE_REALM = "SIPXCHANGE_REALM";

    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Location of the config.defs file. Filled by the constructor of this class
    private String      globalConfigFileLocation = "";

    // Hashtable containing the name value pairs of the definitions defined in config.defs 
    // Populated by the constructor of this class (when an object is instantiated)
    private Hashtable   configValueHash  = new Hashtable();

    // Contains the list of definition names defined in the config.defs files
    // The order in which the definitions are defined is maintained.
    // Populated by the constructor of this class (when an object is instantiated)
    private Vector      configNameVector = new Vector();

    LogUtility logger = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    public GlobalConfigUIHelper()
    {
        this(PathLocatorUtility.getGlobalConfigFileLocation());
    }


    public GlobalConfigUIHelper(String fileName)
    {
        logger = new LogUtility();

        // Get the location of the config.defs file
        globalConfigFileLocation =  fileName;

        // Parse the definitions file the first time an object is instantiated.
        parseGlobalDefinitionsFile();
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /** Checks if the specified definition is defined in config.defs file.
     *  This method is called when other server config files reference the 
     *  global definitions. 
     * 
     * @param  definitionName    Name of the definition that needs to be checked
     * @return boolean          True - if the specified definition name is defined in config.defs
     */ 
    public boolean isDefinedInConfigDefs( String definitionName )
    {
        // Set the return flag to false by default
        boolean isDefined = false;

        if( !configNameVector.isEmpty() )
        {
            // Check if the definitionName exists in the list.
            if( configNameVector.contains( definitionName ) )
                isDefined = true;
        }

        return isDefined ;

    }


    /** Parses the config.defs file and generates the XML representation of it.
     *  A stylesheet is applied on this generated XML and the HTML for the UI is generated.
     * 
     * @return  String containing the generated XML content.
     */ 
    public String generateXML()
    {
        String errorMessage = "";

        // 1. Add the root element - <details>
        StringBuffer xmlContent = new StringBuffer("<definitions>");

        // 2. Check if we were able to successfully parse the config file
        if(  configNameVector != null && 
             !configNameVector.isEmpty() &&
             configValueHash != null &&
             !configValueHash.isEmpty()
        )
        {
            // Semicolon separated list of definition names.
            // This helps preserve the order in which the definitions are listed
            String definitionlist = "";

            // 3. Get the definition name and the associated value
            for( int i=0; i < configNameVector.size(); i++ )
            {
                String key = (String) configNameVector.elementAt(i);
                if( key != null )
                {
                    key = key.trim();
                    definitionlist += key + ";";
                    
                    String value = (String) configValueHash.get(key);
                    if( value == null )
                        value = "";
                    else
                        value = value.trim();

                    // 4. Construct the XML
                    // For each definition, the XML contains:
                    // name -- name of the definition
                    // value -- value associated with the definition
                    // id -- unique Id used by Javascript for dynamically 
                    // inserting a new definition and deleting an existing definition using the UI.
                    xmlContent.append( "<definition>" );
                    xmlContent.append( "<name><![CDATA[" + key + "]]></name>\n" );
                    xmlContent.append( "<value><![CDATA[" + value + "]]></value>\n" );
                    xmlContent.append( "<id>" + i + "</id>\n" );
                    xmlContent.append( "</definition>" );
                }
                else
                {
                    // Ignore. Proceed to the next definition(key).
                }
            }
            xmlContent.append( "<definitionlist><![CDATA[" +  definitionlist + "]]></definitionlist>\n" );
        }
        else
        {
            // Failed to parse the config.defs file
            // errorMessage = ErrorMessageBuilder.getErrorMessage( ErrorMessageBuilder.GLOBAL_CONFIG_FILE_CORRUPTED );
            errorMessage = "Error: Global definitions file " + globalConfigFileLocation + " does not exist or is corrupted. " +
                "Please replace it with the 'config.defs.default' file in the same directory." ;
            xmlContent.append( ErrorMessageBuilder.formatErrorMessage(errorMessage, true, true) );
        }

        xmlContent.append( "</definitions>" );
        return xmlContent.toString();

    }


    /** Generates the XML for constructing the UI based on the values entered by the user.
     *  A stylesheet is applied on this generated XML and the HTML for the UI is generated.
     * 
     *  This method is called when the user has made changes to the definition values but
     *  writing to config.defs failed. The cause of failure could be incorrect values,
     *  and hence the form is re-populated with the values entered by the user along with
     *  the error message.
     * 
     * @param   userEnteredValuesHash   Hashtable containing the definition names and values 
     *                                  entered by the user.
     * @param   errorMessage            String describing the reason for failing to save changes
     * 
     * @return  String                  Contains the generated XML content.
     */ 
    public String generateXML( Hashtable userEnteredValuesHash,
                               String errorMessage)
    {
        // 1. Add the root element - <details>
        StringBuffer xmlContent = new StringBuffer("<definitions>");

        // 2. Get the definition list
        String definitionList = (String) userEnteredValuesHash.get( "definitionlist" );
        StringTokenizer definitionEnumForValidation = new StringTokenizer( definitionList, ";");
        int id = 0 ;
        while( definitionEnumForValidation.hasMoreElements())
        {
            String definition = definitionEnumForValidation.nextToken() ;
            // Check if the definition in the list is available (user may have deleted it)
            if( userEnteredValuesHash.containsKey( definition ) )
            {
                String value = (String) userEnteredValuesHash.get( definition );
                if( value != null )
                    value = value.trim();
                else
                    value = "";
                xmlContent.append( "<definition>" );
                xmlContent.append( "<name><![CDATA[" + definition + "]]></name>\n" );
                xmlContent.append( "<value><![CDATA[" + value + "]]></value>\n" );
                xmlContent.append( "<id>" + id + "</id>\n" );
                xmlContent.append( "</definition>" );
                id ++;
            }
        }
                    
        xmlContent.append( "<definitionlist><![CDATA[" +  definitionList + "]]></definitionlist>\n" );
        xmlContent.append( ErrorMessageBuilder.formatErrorMessage(errorMessage, false, true) );
        xmlContent.append( "</definitions>" );
        return xmlContent.toString();

    }



    /** Helper method for restoring the config.defs file to the default values that were shipped.
     *  SIPxchange is shipped with:
     *  1. config.defs file - editable using the UI
     *  2. config.defs.defaults file. 
     *  When the administrator wants to discard all the changes he made and revert to the original version, 
     *  contents of config.defs file is replaced with that of config.defs.defaults
     *
     * @return String containing the error message if any.
     */ 
    public String restoreToDefaults()
    {
        // Return string
        String errorMessage = "";

        if( globalConfigFileLocation != null && globalConfigFileLocation != "-1" )
        {
            // 1. Get the location of the default config file
            String defaultConfigFileLocation= PathLocatorUtility.getDefaultConfigFileLocation( globalConfigFileLocation );

            // 2. Check if the default config file exists
            File defaultFile = new File( defaultConfigFileLocation );
            if( defaultFile.exists() )
            {
                // 3. Delete the config file and make a copy of the default config file
                File configFile = new File( globalConfigFileLocation );
                configFile.delete();

                // Copy the contents of the default file into the config.defs file
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
                    // error trying to copy the contents
                    //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.UNABLE_TO_WRITE_TO_CONFIG_FILE );
                    errorMessage += "Error: Restore to defaults failed. Cannot write to " + XMLSupport.encodeUtf8(globalConfigFileLocation) + " file. Please verify that this file has write privileges." ;
                    logger.writeToLog( "Restore to defaults failed. Unable to write to " + globalConfigFileLocation );
                }
                
            }
            else
            {
                // default config does not exist
                //errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.DEFAULT_CONFIG_FILE_NOT_FOUND );
                errorMessage = "Error: Restore to defaults failed. File " + XMLSupport.encodeUtf8(defaultConfigFileLocation) + " not found.";
                logger.writeToLog( "Restore to defaults failed. Default config file does not exist at " + defaultConfigFileLocation );
            }
        }
        else
        {
            // failed to obtain the file locations
        }

        return errorMessage;
    }

    /**
     * Replaces current properties (some of them) with the values found in the partial map.
     * And writes the entire set to the file.
     * It's different from writeToConfig which only writes the values that are passed in its parameter.
     * @param partial subset of properties to be replaced 
     */
    public String replaceAndWrite( Map partial )
    {
        StringBuffer definitionlist = new StringBuffer();
        for ( Iterator i = configNameVector.iterator(); i.hasNext(); ) {
            String name = (String) i.next();
            definitionlist.append( name );
            if( i.hasNext() )
            {
                definitionlist.append( ';' );
            }
        }
        Hashtable newConfig = (Hashtable) configValueHash.clone();
        newConfig.put( "definitionlist", definitionlist.toString() );
        newConfig.putAll( partial );
        return writeToConfigFile( newConfig );
    }

    /** Saves the changes made to the config file using the UI
     *  This method:
     *  1. Does a topological sort of the definitions so that the definitions are defined before they are referenced
     *  2. If sorting was fine (that is, no cyclic references were found), writes the data to the config file.
     * 
     * @param userEnteredValuesHash     Hashtable containing the definitions (keys) and the values entered by the user.
     * @return String                   Error message if any.
     */ 
    public String writeToConfigFile( Hashtable userEnteredValuesHash )
    {
        // Return string
        String errorMessage = "";

        // 1. Create a new hashtable from the one passed in as the parameter.
        Hashtable   unsortedDefinitionsHash = new Hashtable( userEnteredValuesHash );

        // 2. Get the "definitionlist" parameter. 
        // This contains a semi-colon separated list of definition names as displayed on the UI.
        // This parameter is helpful because the order of the parameters is lost when retrieving them 
        // from the request object and hence we can no longer group the related parameters together - 
        // more for the cosmetic reason than the functional cause.
        String definitionList = (String) unsortedDefinitionsHash.get( "definitionlist" );

        // 3. Check for well-formedness of the value
        StringTokenizer definitionEnumForValidation = new StringTokenizer( definitionList, ";");
        while( definitionEnumForValidation.hasMoreElements())
        {
            String definition = definitionEnumForValidation.nextToken() ;
            // Check if the definition in the list is available (user may have deleted it)
            if( unsortedDefinitionsHash.containsKey( definition ) )
            {
                String value = (String) unsortedDefinitionsHash.get( definition );
                if( value != null )
                    value = value.trim();
                else
                    value = "";
    
                if( !definitionValueIsValid( value, unsortedDefinitionsHash ) )
                {
                    if( errorMessage.length() == 0 )
                        errorMessage = ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.INVALID_DEFINITION_VALUE );
                    errorMessage += definition + "<br/>" ;
                }
            }
        }
                    

        // 4. Proceed only if all the values are well formed
        if( errorMessage.length() == 0 )
        {

            // Vector for storing the sorted definition names.
            Vector      sortedDefinitionsVector = new Vector();

            // Flag to indicate if a cyclic reference was detected. 
            // When set to true, stop processing and display an error
            boolean     continueProcessing = true;

            StringTokenizer definitionEnum = new StringTokenizer( definitionList, ";");
    
            // 5. Do a topological sort of the definitions.
            while( definitionEnum.hasMoreElements() && continueProcessing)
            {
                // 6. Get the definition name
                String definition = (String) definitionEnum.nextElement();
                if( unsortedDefinitionsHash.containsKey( definition ) )
                {
                    // 4b. Get the value associated with the definition
                    String value = (String) unsortedDefinitionsHash.get( definition );
                    if( value != null )
                        value = value.trim();
                    else
                        value = "";
        
                    // 5. Check if this value references any other definition.
                    // If yes, check for cyclic reference
                    // If no, add to the sorted vector
                    if( value.indexOf("${") == -1 )
                    {
                        // 5a. If not already available, add it to the sorted vector
                        if( !sortedDefinitionsVector.contains( definition ) )
                            sortedDefinitionsVector.addElement(definition);
                    }
                    else
                    {
                        // 5b. Check if the definition value references more than one definitions
        
                        // Vector for storing the definitions referenced by this definition
                        Vector  nodes = new Vector();
                        // 6. check if there is a loop in definition references
                        if( !definitionLoopFound(value, unsortedDefinitionsHash, nodes) )
                        {
                            // No loop found.
                            int iterator = nodes.size() -1;
                            // 7a. Add the definitions referenced to the sorted vector
                            for(; iterator >= 0; iterator-- )
                            {
                                String temp = (String) nodes.elementAt( iterator ) ; 
                                
                                if( !sortedDefinitionsVector.contains( temp ) )
                                    sortedDefinitionsVector.addElement( temp ) ;
                            }
        
                            
                            // 7b. Finally add the definition under consideration to the sorted vector
                            if( !sortedDefinitionsVector.contains( definition ) )
                                sortedDefinitionsVector.addElement( definition );
        
                        }
                        else                    
                        {
                            // Loop detected. Stop processing.
                            continueProcessing = false ;
        
                            // Generate the error message
                            errorMessage += ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.CYCLIC_DEFINITION_REFERENCE );
                            for( int j=0; j<nodes.size(); j++)
                                errorMessage += nodes.elementAt(j) + "<br/>" ;
                            
                        }
                    }
                }
            }

            if( continueProcessing )
            {
                // Save the sorted definitions
                StringBuffer fileContentsBuffer = new StringBuffer();
                for( int k=0; k<sortedDefinitionsVector.size(); k++)
                {
                    String definitionName = (String) sortedDefinitionsVector.elementAt(k) ;
                    String value = (String) unsortedDefinitionsHash.get( definitionName );
                    if( value != null )
                        value = value.trim();
                    else
                        value = "";

                    fileContentsBuffer.append(definitionName + "=" + value + "\n");
                }
                if( fileContentsBuffer.length() > 0 )
                {
                    try 
                    {
                        BufferedWriter out = new BufferedWriter(new FileWriter(globalConfigFileLocation));
                        out.write(fileContentsBuffer.toString());
                        out.close();
                
                    } catch ( Exception e ) {
                        //errorMessage += ErrorMessageBuilder.getErrorMessage ( ErrorMessageBuilder.UNABLE_TO_WRITE_TO_CONFIG_FILE );
                        errorMessage += "Error: Cannot write to " + XMLSupport.encodeUtf8(globalConfigFileLocation) + " file. Please verify that this file has write privileges." ;
                    }

                }
            }
        }
        
        return errorMessage;
    }


    public String getDefinitionList( String generatedXML )
    {
        String definitionList = "" ;
        int elementIndex = generatedXML.indexOf("<definitionlist>");
        if( elementIndex != -1 )
        {
            int startIndex = generatedXML.indexOf( "CDATA[", elementIndex);
            if( startIndex != -1 )
            {
                startIndex += 6 ;
                int endIndex = generatedXML.indexOf( "]]", startIndex );

                if( endIndex != -1 )
                    definitionList = generatedXML.substring(startIndex,endIndex);

            }
        }
        return definitionList ;
    }



//////////////////////////////////////////////////////////////////////////////
// Private Methods
////

    private void parseGlobalDefinitionsFile()
    {
        // 1. Read the file and extract the properties and the value.
        if ( globalConfigFileLocation != null && globalConfigFileLocation != "-1") {
            try {
                BufferedReader in = new BufferedReader(new FileReader(globalConfigFileLocation));
                String str;
                while ((str = in.readLine()) != null) {
                    // If the line starts with #, ignore it.
                    str = str.trim();
                    if( !str.startsWith("#") )
                    {
                        // Parse each line to separate key (configuration name) and data
                        int separatorIndex = str.indexOf('=');
                        if ( separatorIndex != -1) {
                            String key = str.substring( 0, separatorIndex).trim();
                            String value = "";
                            if ( separatorIndex < (str.length()-1) )
                                value = str.substring(separatorIndex+1).trim();
    
                            // Add a entry to the hash table.
                            configValueHash.put(key,value);
                            configNameVector.add(key);
                        }
                    }
                }
                in.close();
            } catch (IOException ie ) {
                logger.writeToLog( "Failed to read " +  globalConfigFileLocation );
            }
        } else {
            // Failed to retrieve the location of the config file
            logger.writeToLog( "Failed to retrieve the location of config.defs" );

        }

        return;
    }


    /** Helper method for determining if there is a cyclic reference between the definitions.
     * 
     * @param value                     Definition value that needs to be checked
     * @param unsortedDefinitionsHash   Hashtable containing the definitionname -value pairs
     * @param nodes                     Vector for storing the names of definitions referenced. 
     *                                  Filled on return.
     * 
     * @return int                      Provides different status.
     */
    private boolean definitionLoopFound( String value,
                                         Hashtable unsortedDefinitionsHash,
                                         Vector nodes
                                         )
    {
        boolean loopDetected  =  false;

        // 1. Get the location of the first definition referenced.
        int startDefinitionIndex = value.indexOf("${");
        while( startDefinitionIndex > -1 && !loopDetected)
        {
            // 2. Get the name of the definition
            int endDefinitionIndex = value.indexOf("}", startDefinitionIndex );
            String referencedDefinition = value.substring(startDefinitionIndex+2, endDefinitionIndex);

            boolean checkReferences = true;
            while( checkReferences )
            {
                // Check if the referenced Definition is already available in our vector
                if ( nodes.contains(referencedDefinition) ) {
                    checkReferences = false;
                    loopDetected = true;
                }
                else
                {
                    // 3. Add the referenced definition to the vector
                    nodes.addElement(referencedDefinition);
    
                    // 4. Get the value of the referenced definition
                    String referencedDefinitionValue = (String) unsortedDefinitionsHash.get( referencedDefinition );
                    if( referencedDefinitionValue != null )
                        referencedDefinitionValue = referencedDefinitionValue.trim();
                    else
                        referencedDefinitionValue = "";
    
                    // 5. Check if it refers to another definition
                    if( referencedDefinitionValue.indexOf("${") == -1 )
                    {
                        // No loop detected. 
                        checkReferences = false ;
                    }
                    else
                    {
                        if( definitionLoopFound(referencedDefinitionValue, unsortedDefinitionsHash, nodes) )
                        {
                            loopDetected    = true ;
                            checkReferences = false ;
                        }
                        else
                        {
                            // Add the names of the definitions referenced to our vector.
                            /*for( int j=0; j<childNodes.size(); j++)
                                nodes.addElement(childNodes.elementAt(j));*/
                            checkReferences = false ;
                        }
                    }
                }
            }
            // Get the next definition referenced by the value
            startDefinitionIndex = value.indexOf("${", endDefinitionIndex);
        }

        return loopDetected ;
    }

    private boolean definitionValueIsValid( String value, Hashtable unsortedDefinitionsHash )
    {
        // Validation Rules:
        // 1. For every '${' there should be a corresponding '}'
        // 2. Nested definitions not allowed. That is, ${...${..}..}.

        boolean isValid = false;

        if( value.indexOf("${") > -1 )
        {
            int startIndex = value.indexOf("${");
            boolean continueProcessing = true ;
            while( startIndex > -1 && continueProcessing)
            {
                int endIndex = value.indexOf("}", startIndex + 2)  ;
                if( endIndex > startIndex )
                {
                    // There is a corresponding end tag.
                    // Check if the referenced definition exists
                    String referencedDefinition = value.substring(startIndex+2,endIndex);
                    if( unsortedDefinitionsHash.containsKey( referencedDefinition ) )
                    {
                        // Check for nesting.
                        int nestedIndex = value.indexOf("${", startIndex+2);
                        if( nestedIndex != -1 )
                        {
                            // Next '${' is present after the previous '}'. No nesting!
                            if( nestedIndex > endIndex )
                                startIndex = nestedIndex;
                            else
                                continueProcessing = false ;
                        }
                        else
                        {
                            isValid = true;
                            continueProcessing = false ;
                        }
                    }
                    else
                    {
                        continueProcessing = false ;
                    }
                }
                else
                {
                    // There is no corresponding end tag
                    continueProcessing = false;
                }
            }
        }
        else if( value.indexOf("`") != -1 )
        {
            int startIndex = value.indexOf("`");
            int endIndex = value.indexOf("`", startIndex+1);
            if( endIndex > startIndex )
                isValid = true;
        }
        else
        {
            isValid = true;
        }
        return isValid;

    }


    public static void main(String[] argv)
    {
        GlobalConfigUIHelper helper = new GlobalConfigUIHelper();
        Hashtable myhash = new Hashtable (helper.configValueHash);
        // Add a new key to indicate the order of retrieval.
        StringBuffer definitionlist = new StringBuffer();
        for( int i=0; i< helper.configNameVector.size(); i++)
            definitionlist.append( helper.configNameVector.elementAt(i) + ";");
        myhash.put( "definitionlist", definitionlist.toString() );
        helper.writeToConfigFile( myhash );

    }
}

