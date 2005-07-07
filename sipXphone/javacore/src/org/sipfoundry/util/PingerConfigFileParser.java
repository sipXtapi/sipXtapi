/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.util;

import org.sipfoundry.sipxphone.sys.util.PingerInfo ;

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.io.*;

public class PingerConfigFileParser
{

    /**
     * Load configuration file from disk
     */
    public static OrderedHashtable getConfigFile(String fileName) throws IOException
    {
        OrderedHashtable table = new OrderedHashtable();

        try
        {
          byte[] buffer = loadConfigDbFromFile(fileName);
          if  (buffer == null)
              throw new IOException("Could not load buffer");

          Reader isr = new InputStreamReader(new ByteArrayInputStream(buffer));
          BufferedReader reader = new BufferedReader(isr);
          String	str;

          while ( (str = reader.readLine()) != null)
          {
              str = str.trim() ;
              int firstColon = str.indexOf(":");

              if (firstColon > 0)
              {
                  String key = str.substring(0, firstColon - 1);
                  key = key.trim();
                  String value = str.substring(firstColon + 1);
                  value = value.trim();
                  table.put(key, value);
              }
          }//end while
          reader.close();
        }
        catch (IOException e)
        {
          throw new IOException(fileName + " not found");
        }

        return table;
    }


    public static void setConfigFile(String strFilename, OrderedHashtable configTable) throws IOException
    {
        try {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            BufferedWriter os = new BufferedWriter(new OutputStreamWriter(buffer));
            String	strLine = "";
            Enumeration eKeys = configTable.keys() ;
            Enumeration eElements = configTable.elements() ;

            while ( eKeys.hasMoreElements() )
            {
                    strLine = eKeys.nextElement() + " : " + eElements.nextElement() + "\r\n" ;
                    os.write(strLine) ;
            }
            os.flush() ;
            os.close() ;
            if (!storeConfigDbToFile(strFilename, buffer.toByteArray()))
                throw new IOException("Could not save file");

		    }

		    catch (IOException e)
		    {
			      throw new IOException
                ("PingerConfigFileParser::setConfigFile(), could not save to " + strFilename);
		    }
    }

   /**
    * Get a value from the pinger-config for a particular key
    * @param strConfig name of the config file
    * @param strKey name of the
    */
    public static String getValue(String strConfig, String strKey)
    {
        String strValue =  "" ;
        try {
            strValue = (String) getConfigFile(strConfig).get(strKey) ;
        } catch (IOException e) {
            System.out.println("PingerConfigFileParser::getValue() Error reading from " + strConfig) ;
            SysLog.log(e) ;
        }
        return strValue ;
    }


    /**
     * This method writes a key/value pair to the pinger-config file.
     * If the key already exists, it will overwrite the current value.
     *
     * @param strConfig, the name of the config file (e.g. pinger-config)
     * @param strKey, the key to be written to pinger-config
     * @param strValue the key value to be written to pinger-config.  A null
     *        value will remove the key.
     */
     public static void setValue(String strConfig, String strKey, String strValue)
     {
        try {
            String prefixPathToConfigFiles = PingerInfo.getInstance().getFlashFileSystemLocation();

            String fullConfigFileName = prefixPathToConfigFiles + File.separator + strConfig;

            OrderedHashtable hashConfig = null ;
            try {
                // Read in everything into a hashtable
                hashConfig = getConfigFile(fullConfigFileName) ;
            } catch (IOException ioe) {
                System.out.println("io exception reading: " + fullConfigFileName) ;
                SysLog.log(ioe) ;
                hashConfig = new OrderedHashtable() ;
            }

            // Add to pinger-config, overwriting an existing key if there is one
            if (strValue != null) {
                hashConfig.put(strKey, strValue) ;
                PingerConfig.getInstance().getConfig().put(strKey, strValue) ;
            } else {
                hashConfig.remove(strKey) ;
                PingerConfig.getInstance().getConfig().remove(strKey) ;
            }

    	    // Save new pinger-config file
    	    setConfigFile(fullConfigFileName, hashConfig) ;

        } catch (IOException e) {
            System.out.println("PingerConfig::setValue failed") ;
    	    SysLog.log(e) ;
    	}
     }


      /**
       * <pre>
     *     If the config file has lines in the following structure,
     *     USER_LINE.1.ALLOW_FORWARDING : DISABLE
     *     USER_LINE.1.REGISTRATION : PROVISION
     *     USER_LINE.1.URL : sip:6666@10.1.1.194
     *
     *     USER_LINE.2.ALLOW_FORWARDING : ENABLE
     *     USER_LINE.2.REGISTRATION : PROVISION
     *     USER_LINE.2.URL : sip:6466@10.1.1.104
     *
     *
     *    It returns you a hastable of "objectID and fieldHash" for
     *    the keygroup you are looking for.
     *    eg to find the value of fields of USER_LINE.2, you need to do this:
     *
     *     Hashtable hashObjects = getGroupValue("USER_LINE");
     *     if( hashObjects != null ){
     *          Hashtable fieldsHash  = hashObjects.get("2");
     *          //And to find the value of URL field of USER_LINE.2,
     *          if( fieldsHash != null )
     *              String url = fieldsHash.get("URL");
     *     }
     *
     *     String groupkey | Hashtable  objectsHash|
     *                     |  String    keyObjectID|   Hashtable  fieldsHash    |
     *                                             |   fieldname  | fieldvalue  |
     *    <pre>
     *    htConfigGroupData will have elements added. We need to pass the hashtable in
     *    as we need to check if there is already a objectsHash in it.
    */
     public static void handleGroupData(Hashtable htConfigGroupData, String key, String value)
     {
          key = key.trim();
          value = value.trim();

          int dotSeparator = key.indexOf(".");
          if( dotSeparator != -1 ){
            StringTokenizer tokenizer = new StringTokenizer(key, ".");
            String keyObjectGroupName = null;
            String keyObjectID = null;
            String keyObjectFieldName = null;

            if( tokenizer.hasMoreTokens() ){
                keyObjectGroupName    = tokenizer.nextToken();
            }
            if( tokenizer.hasMoreTokens() ){
                keyObjectID  = tokenizer.nextToken();
            }
            if( tokenizer.hasMoreTokens() ){
                keyObjectFieldName  = tokenizer.nextToken();
            }
            //this grouping is only valid for line like
            //USER_LINE.1.ALLOW_FORWARDING : DISABLE
            //USER_LINE.1.REGISTRATION : PROVISION
            //USER_LINE.1.URL : sip:6666@10.1.1.194
            if( ( keyObjectGroupName != null ) &&
                ( keyObjectID        != null ) &&
                ( keyObjectFieldName != null ) &&
                ( value              != null )  ){

                    //keyObjectGroupName is USER_LINE in the above example
                    Hashtable objectsHash =
                            (Hashtable)htConfigGroupData.get( keyObjectGroupName );

                    if( objectsHash == null ){
                        objectsHash = new Hashtable();
                    }
                    //keyObjectID is 1 in the above example

                    Hashtable fieldsHash =
                                ( Hashtable) objectsHash.get( keyObjectID);

                    if(  fieldsHash == null ){
                        fieldsHash = new Hashtable();
                    }
                    fieldsHash.put ( keyObjectFieldName,  value       );
                    objectsHash.put( keyObjectID,         fieldsHash  );
                    //System.out.println("putting key " + keyObjectFieldName);
                    //System.out.println("putting value " + value);
                    htConfigGroupData.put (keyObjectGroupName,   objectsHash );
            }
         }
    }

    private static native byte[] loadConfigDbFromFile(String filename);

    private static native boolean storeConfigDbToFile(String filename, byte[] buffer);

}


