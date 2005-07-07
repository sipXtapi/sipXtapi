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


package org.sipfoundry.sipxphone.sys.directoryservice.provider;

import javax.naming.directory.*;
import javax.naming.*;
import java.util.*;
import java.io.InputStream;
import java.io.IOException;

import org.sipfoundry.util.*;
import org.sipfoundry.sipxphone.sys.*;

/**
 * This class is the Directory Service Provider (DSP) for the Speed Dial service.
 */
public class SpeedDialDSP extends DirectoryServiceProvider
{
    // This section is used as a hint as to what the schema is for this directory
    // In the future, this would get captured in the SchemaMap

    public static final String SPEED_DIAL_SCHEMA_LABEL = "label";
    public static final String SPEED_DIAL_SCHEMA_PHONE_NUMBER = "phone_number";
    public static final String SPEED_DIAL_SCHEMA_URL = "url";
    public static final String SPEED_DIAL_SCHEMA_ID = "id";



    protected Context m_initialContext = null;

    protected String m_strDataResourceFile;

    /** Reference to singleton instance */
    private static SpeedDialDSP m_reference = null ;


    private SpeedDialDSP()
    {
        boolean loaded = false;

        // Set up to use JNDI
        Hashtable env = new Hashtable(5, 0.75f);
        env.put("java.naming.factory.initial",
            "org.sipfoundry.sipxphone.sys.jndi.FlatInitCtxFactory");

        try {
            m_initialContext = new InitialContext(env);
        } catch (Exception e) {
            System.out.println("SpeedDialDSP: can't init jndi!");
        }

        // This is for debugging purposes. It dumps out the contents of the phone book.
        // dump();


        // So. what the heck is this hack for? Well, with 0.7.0, the schema
        // used for the speed dial entries has changed. We first determine
        // if this is the first time that they have run 0.7.0. If so, then we
        // whack the old speed dial directory
        Repository repository = Repository.getInstance();

        String oldVersion = repository.getOldRepositoryVersion();
        String newVersion = repository.getNewRepositoryVersion();

        if (oldVersion != null &&
            newVersion  != null &&
            newVersion.compareTo("0.7.0") == 0 &&
            newVersion.compareTo(oldVersion) != 0)
        {
            System.out.println("SpeedDialDSP:: This is the first time 0.7.0 has been run. Whack the old speed dial entries.");

            this.clear();
        }


    }

    /**
     *Is the speeddial datasource local( from cache.ser )
     *or from config files which get written by config server?
     */
    public boolean isDataSourceLocal(){
        boolean bLocal = true;
        PingerConfig config = PingerConfig.getInstance() ;
        String strMaxCallLogEntries = config.getValue("PHONESET_SPEEDDIAL_DATASOURCE") ;
        if(    ( strMaxCallLogEntries != null )
            && (strMaxCallLogEntries.equals("CONFIG")) ){
            bLocal = false;
        }
        return bLocal;
    }

    public void setDataResourcesFile( String strDataResourceFile )
    {
        m_strDataResourceFile = strDataResourceFile;
    }

    /**
     * For now, if the datasource is not local and read from config files,
     * due to the limitation of one-way comunication between config files and
     * config server, the data-source will be read-only.
     */
    public boolean isDataSourceReadOnly(){
        boolean bRet = false;
        if( !isDataSourceLocal() )
            bRet = true;
        if( m_strDataResourceFile != null)
            bRet = true;
        return bRet;
    }

    public static SpeedDialDSP getInstance()
    {
        // Create if needed
        if (m_reference == null) {
            m_reference = (SpeedDialDSP) new SpeedDialDSP();
        }

        return m_reference ;
    }

    /**
     * This method clears the directory, removing every entry in the directory.
     */
    public void clear() throws IllegalArgumentException
    {
        try {
              NamingEnumeration bindingsEnum = m_initialContext.listBindings("");
              while (bindingsEnum.hasMore()) {
                  Binding binding = (Binding) bindingsEnum.next();
                  m_initialContext.unbind(getSpeedDialId((BasicAttributes)binding.getObject()));
              }
        } catch (Exception e) {
            SysLog.log(e);
                throw new IllegalArgumentException("Error clearing phonebook");

        }
    }

    public void addEntry(BasicAttributes attrs)
    {

        // Try to write out the entry. If it already exists, an exception is thrown
        try {
            m_initialContext.bind(getSpeedDialId(attrs), (Object) attrs);
        } catch (Exception e) {

            // It looks like the entry is there already. Write over it.
            try {
                m_initialContext.rebind(getSpeedDialId(attrs), (Object) attrs);
            } catch (Exception ex) {
                // This should never happen
                System.out.println("Error in addEntry: " ) ;
                SysLog.log(e) ;
                // Shell.getInstance().showMessageBox("Warning", "\nError saving phonebook\n\n" + e) ;
            }
        }
    }

    public void deleteEntry(BasicAttributes attrs) throws IllegalArgumentException
    {
        // Try to erase out the entry. If t does not exist, throw an exception.
        try {
            m_initialContext.unbind(getSpeedDialId(attrs));
        } catch (Exception e) {
            throw new IllegalArgumentException("Key not found");
        }
    }

    public BasicAttributes lookupName(String name) throws IllegalArgumentException
    {
        //TODO:
        return null;
        /*
        BasicAttributes attrs = null;

        // Try to find out the entry.
        try {
            attrs = (BasicAttributes) m_initialContext.lookup(name);
        } catch (Exception e) {
            throw new IllegalArgumentException("Key not found");
        }

        return attrs;
        */
    }

    /**
     * This method is used to seatch for a speed dial ID. If the entry is found,
     * it is returned, otherwise an exception is thrown.
     */
    public BasicAttributes lookupSpeedDialId(String id) throws IllegalArgumentException
    {

        Vector entries = getEntries();

        if (entries.isEmpty())
            System.out.println("Empty!");

        Enumeration enum = entries.elements();

        while (enum.hasMoreElements()) {
            BasicAttributes attr = (BasicAttributes) enum.nextElement();

            if (getSpeedDialId(attr).compareTo(id) == 0)
                return attr;
        }

        throw new IllegalArgumentException("Key not found");
    }

    /**
     * Look up an entry in the phone book given the caller ID.
     */
    public BasicAttributes lookupCallerID(String callerID) throws IllegalArgumentException
    {
        // TODO:
        return null;
    }


    /**
     * gets the SpeedDial entries by reading from a property file. SpeedDial
     * Data should be in the form of
     *  PHONESET_SPEEDDIAL.1.ID=1
        PHONESET_SPEEDDIAL.1.LABEL=roomspeed1
        PHONESET_SPEEDDIAL.1.TYPE=number
        PHONESET_SPEEDDIAL.1.ADDRESS=222222
        If the propertyf filename is pinger-config, then gets the already populated data
        from PingerConfig.
     */
    public Vector getEntriesFromDataFile(String strDataSourceFile)
    {

        Vector vEntries = new Vector();
        Hashtable hashObjects = null;
        if( strDataSourceFile.equals(PingerConfig.PINGER_CONFIG) )
        {
            //read from the pinger-config/user-config file.
            hashObjects =
                PingerConfig.getInstance().getGroupValue("PHONESET_SPEEDDIAL");
        }else
        {
            Hashtable hashGroupConfigData = new Hashtable();
            try {
                InputStream stream =
                    ClassLoader.getSystemResourceAsStream(strDataSourceFile);
                if( stream != null )
                {
                    ResourceBundle resourceBundle = new PropertyResourceBundle(stream) ;
                    Enumeration keysEnum   = resourceBundle.getKeys();
                    while (keysEnum.hasMoreElements() )
                    {
                        String key   = (String) keysEnum.nextElement();
                        String value = resourceBundle.getString(key);
                        PingerConfigFileParser.handleGroupData( hashGroupConfigData, key, value);
                    }
                }
            }catch(IOException e )
            {
                SysLog.log(e);
            }
            hashObjects = (Hashtable)( hashGroupConfigData.get("PHONESET_SPEEDDIAL"));
        }

        if( hashObjects != null )
        {
           Enumeration fieldsHashEnum  = hashObjects.elements();
           int count=1;
           while( fieldsHashEnum.hasMoreElements() ){
               Hashtable fieldHash = (Hashtable)fieldsHashEnum.nextElement();
               BasicAttributes attrs = new BasicAttributes();
               String id = (String)fieldHash.get("ID");
               String label =(String)fieldHash.get("LABEL");
               String type = (String)fieldHash.get("TYPE");
               String address =(String)fieldHash.get("ADDRESS");

               if( (id != null) && (label != null) &&  (type != null)
                                && (address != null ) ){
                   attrs.put(SPEED_DIAL_SCHEMA_ID, id);
                   attrs.put(SPEED_DIAL_SCHEMA_LABEL, label );
                   if( type.equals("url") ){
                      attrs.put(SPEED_DIAL_SCHEMA_URL, address);
                   }else{
                      attrs.put(SPEED_DIAL_SCHEMA_PHONE_NUMBER, address);
                   }
                   vEntries.addElement(attrs);
                }
           }
        }
        return  (sortEntries(vEntries, SPEED_DIAL_SCHEMA_ID, true)) ;

    }


    /**
     * This method returns a list of all of the entries in the directory.
     * If the data source is local, it reads the data from cache.ser,
     * otherwise it reads them from the pinger-config/user-config or
     * m_strDataResourceFile if not null. The data in the property file
     * should be in the form of
     *  PHONESET_SPEEDDIAL.1.ID=1
        PHONESET_SPEEDDIAL.1.LABEL=roomspeed1
        PHONESET_SPEEDDIAL.1.TYPE=number
        PHONESET_SPEEDDIAL.1.ADDRESS=222222
     */
    public Vector getEntries()
    {
        Vector vSorted = null;
        try {
            if( m_strDataResourceFile != null )
            {
                vSorted = getEntriesFromDataFile( m_strDataResourceFile );
            }else
            {
                if( isDataSourceLocal() ){
                    Vector vEntries = new Vector();
                    NamingEnumeration bindingsEnum = null;
                    if (m_initialContext != null)
                    {
                    	bindingsEnum = m_initialContext.listBindings("");
                    }
                    if (bindingsEnum != null)
                    {
	                    while (bindingsEnum.hasMore()) {
	                        Binding binding = null;
	                        binding = (Binding) bindingsEnum.next();
	                        String bindingName = null;
	                        if (binding != null)
	                        {
	                        	bindingName = binding.getName();
		                        BasicAttributes attrs = null;
		                        attrs = (BasicAttributes) binding.getObject();
		                        if (attrs != null)
		                        {
		                        	vEntries.addElement(attrs);
		                        }
	                        }

	                    }
                    }
                    vSorted = sortEntries(vEntries, SPEED_DIAL_SCHEMA_ID, true) ;

                }else{
                    vSorted = getEntriesFromDataFile( PingerConfig.PINGER_CONFIG );
                }
            }

        } catch (Exception e) {
            System.out.println("Error traversing phonebook: " ) ;
            SysLog.log(e) ;
        }
        if( vSorted == null )
            vSorted = new Vector();

        return vSorted;    
    }

    private static String getAttr(BasicAttributes attrs, String attrName)
    {

        Attribute attr = null;

        try {
            if (((attr = attrs.get(attrName)) == null))
            {
                return null;
            } else {
                return (String) attr.get();
            }
        } catch (Exception e) {
            return null;
        }
    }

    private void dumpEntry(BasicAttributes attrs)
    {
        System.out.println(getLabel(attrs));

        Enumeration attrsEnum = attrs.getIDs();

        while (attrsEnum.hasMoreElements()) {
            String attrName = (String) attrsEnum.nextElement();
            String attrValue = null;

            try {
                attrValue = (String) attrs.get(attrName).get();
            } catch (Exception e) {
            }

            System.out.println("    " + attrName + ", " + attrValue);
        }
    }

    /**
     * Dump to stdout the directory information for this object
     */

    public void dump()
    {
        Vector entries = getEntries();

        if (entries.isEmpty())
            System.out.println("Empty!");

        Enumeration enum = entries.elements();

        while (enum.hasMoreElements()) {
            dumpEntry((BasicAttributes) enum.nextElement());
        }
    }

    // These methods are utility methods for this schema. They will move to the
    // schema's class, when I write it.
    public static String getLabel(BasicAttributes attrs)
    {
        return getAttr(attrs, SPEED_DIAL_SCHEMA_LABEL);
    }

    public static String getURL(BasicAttributes attrs)
    {
        return getAttr(attrs, SPEED_DIAL_SCHEMA_URL);
    }

    public static String getSpeedDialId(BasicAttributes attrs)
    {
        return getAttr(attrs, SPEED_DIAL_SCHEMA_ID);
    }

    public static String getSpeedDialPhoneNumber(BasicAttributes attrs)
    {
        return getAttr(attrs, SPEED_DIAL_SCHEMA_PHONE_NUMBER);
    }

    public static String getSpeedDialURL(BasicAttributes attrs)
    {
        return getAttr(attrs, SPEED_DIAL_SCHEMA_URL);
    }

    /** Gets the next Speed Dial ID available
     * If 1,2,3,4, 100 were the assigned IDs,
     * it would return you 5.
     */
    public  int getNextAvailableID() {
        boolean bFound = false;

        Enumeration enum =
            getEntries().elements() ;
        int iNextID = 1;
        try{
            while (enum.hasMoreElements() && !bFound ){
                BasicAttributes data =
                    (BasicAttributes)enum.nextElement();
                int iID =
                    Integer.parseInt(getSpeedDialId(data));
                if( iID == iNextID ){
                    iNextID++;
                }else{
                    bFound = true;
                }
            }
        }catch( Exception e ){
            SysLog.log(e);
        }
        return iNextID;
    }

    private  boolean bNullOrEmpty( String str ){
        boolean bRet = false;
        if( str != null ){
            if( str.trim().length() == 0 )
                bRet = true;
        }else{
            bRet = true;
        }
        return bRet;
    }


  /**
   * Validates the speed dial Entry. Returns an appropriate error
   * message if any of the items are invalid. The conditions are:
   * 1) None of the items can be "null".
   * 2) Speed Dial ID cannot have a leading zero.
   * 3) If it is a SIP URL, it has to be a valid SIP URL.
   * 4) If its a Phone Number, it has to be a valid phone number.
   * 5) Speed Dial ID can only be numeric.
   */
    public  String validateEntry( String strID, String strLabel,
                            String strPhoneNumber, String strURL,
                            boolean bIsPhoneNumber, String strNewLine){

        StringBuffer buffer = new StringBuffer();
        if( bNullOrEmpty( strID ) ){
            buffer.append(" You must enter a speed dial ID." );
            buffer.append(strNewLine);
        }else{
            String trimmedID = strID.trim();
            if( !(trimmedID.equals("0")) &&
                trimmedID.startsWith("0") ){
                buffer.append(" Speed dial ID cannot have leading zeros." );
                buffer.append(strNewLine);
            }
        }
        if( bNullOrEmpty( strLabel ) ){
             buffer.append(" You must enter a label. " );
             buffer.append(strNewLine);
        }
        if( bIsPhoneNumber ){
            if( bNullOrEmpty( strPhoneNumber ) ){
                buffer.append(" You must enter a phone number. " );
                buffer.append(strNewLine);
            }
        }else{
            if( bNullOrEmpty( strURL ) ){
                buffer.append(" You must enter a SIP URL." );
            }else if( strURL.trim().equals("sip:") ){
                buffer.append(" You must enter a valid sip url. ");
            }
            //there is no real "fit all" way of validating a url
            //as a url is context-sensitive, For example, sometimes, a blank host is
            //filled in by the system.  The good analogy is adding a bookmark
            //in Netscape browser, you can add any junk as the URL and it only fails
            //when you actually connect to it.
            //else if( !( strURL.trim().startsWith("sip:") )){
            //    buffer.append(" You must enter a valid sip url. " );
            //}
        }

        if( buffer.length() == 0 ){
            int number = -1;
            try{
                number = Integer.parseInt( strID.trim() );
            }catch( NumberFormatException e ){
                buffer.append
                ("Speed Dial ID can only be a number with no decimal points." );
            }
            if( bIsPhoneNumber ){
                if( ! isValidPhoneNumber( strPhoneNumber ) )
                    buffer.append("You must enter a valid phone number." );
            }
        }
        return buffer.toString();
    }

  /**
  * Is the phone number a valid phone number?
  * If it consists of chars  0,1,2,3,4,5,6,7,8,9, ,+,-,*,p,i
  * then it is valid otherwise it is not.
  */
    protected boolean isValidPhoneNumber(String strPhoneNumber){
        boolean bValid = true;
        char[] charArray = strPhoneNumber.toCharArray();
        for( int i = 0; i<charArray.length; i++ ){
            switch( charArray[i] ){
            case '0' : case '1' : case '2' : case '3' : case '4' :
            case '5' : case '6' : case '7' : case '8' : case '9' :
            case ' ' : case '+' : case '-' : case '*' : case '#' :
            case PhoneNumberParser.CHAR_PAUSE:
            case PhoneNumberParser.CHAR_WAIT:
                break ;
            default: bValid = false;
            }
        }
        return bValid;
    }

    /**
     * get all entries matching the key prefix
     */
    public Vector filterSpeedEntries(String strKey)
    {
        Vector vRC = new Vector();

        try {
            //NamingEnumeration bindingsEnum = m_initialContext.listBindings("");
           // while (bindingsEnum.hasMore()) {
           //     Binding binding = (Binding) bindingsEnum.next();
            Enumeration enum = getEntries().elements();
            while( enum.hasMoreElements() ){

                //BasicAttributes attrs = (BasicAttributes) binding.getObject();
                BasicAttributes attrs = (BasicAttributes)enum.nextElement();

                String label = getSpeedDialId(attrs);

                if (label != null && (label.startsWith(strKey)))
                {
                    vRC.addElement(attrs);
                }
            }
        } catch (Exception e) {
            System.out.println("Error traversing phonebook: " ) ;
            SysLog.log(e) ;
        }

        return vRC;
    }

    /**
     * This method sorts the entries based on the schema field passed in
     */

    public Vector sortEntries(Vector entries, String schemaField, boolean bAscending)
    {
        int numEntries = entries.size();
        Vector newEntries = new Vector(numEntries);

        // Sort for integers, i.e. SPEED_DIAL_SCHEMA_ID
        if (schemaField == SPEED_DIAL_SCHEMA_ID) {
            int keys[] = new int[numEntries];
            int keyNum = 0;

            Enumeration entriesEnum = entries.elements();

            // Pull out the IDs of the entries in the list
            while (entriesEnum.hasMoreElements())
            {
                BasicAttributes attrs = (BasicAttributes) entriesEnum.nextElement();

                keys[keyNum++] = new Integer(getAttr(attrs, schemaField)).intValue();
            }

            //SortIntArray sorter = new SortIntArray(keys) ;
            //int[] sortedKeys = sorter.getSortedArray(true) ;
            QuickSort quickSort = new QuickSort();
            try{
                quickSort.sort(keys);
            }catch( Exception e ){
                SysLog.log(e);
            }
            newEntries.setSize(numEntries);

            // For each of the keys in the list
            for (int i=0; i < numEntries; i++)
            {
                //System.out.println(">>>> i " + i);
                //int id = sortedKeys[i];
                int id = keys[i];

                // Search for the key in the original vector and build a new vector
                for (int j=0; j < numEntries; j++)
                {
                    BasicAttributes searchEntry = (BasicAttributes) entries.elementAt(j);
                    int thisKey = new Integer(getAttr(searchEntry, schemaField)).intValue();
                    // If we found the entry in the vector, copy it over to the new vector

                    if (id == thisKey)
                    {
                        //System.out.println(">>>> thisKey " + thisKey);
                        newEntries.setElementAt(searchEntry, i);
                        break;
                    }
                }

            }



        } else {
            String[] keys = new String[numEntries];

            int keyNum = 0;

            Enumeration entriesEnum = entries.elements();

            // Pull out the IDs of the entries in the list
            while (entriesEnum.hasMoreElements())
            {
                BasicAttributes attrs = (BasicAttributes) entriesEnum.nextElement();

                keys[keyNum++] = (String) getAttr(attrs, schemaField);
            }
            //SortStringArray sorter = new SortStringArray(keys);
            //String[] sortedKeys = sorter.getSortedArray(true);
            QuickSort quickSort = new QuickSort();
            try{
                quickSort.sort(keys);
            }catch( Exception e ){
                SysLog.log(e);
            }
            newEntries.setSize(numEntries);

            // For each of the keys in the list
            for (int i=0; i < numEntries; i++)
            {
                //System.out.println(">>>> i " + i);
                //String id = sortedKeys[i];
                String id = keys[i];

                // Search for the key in the original vector and build a new vector
                for (int j=0; j < numEntries; j++)
                {
                    BasicAttributes searchEntry = (BasicAttributes) entries.elementAt(j);
                    String thisKey = (String) getAttr(searchEntry, schemaField);
                    // If we found the entry in the vector, copy it over to the new vector

                    if (id.compareTo(thisKey) == 0)
                    {
                        //System.out.println(">>>> thisKey " + thisKey);
                        newEntries.setElementAt(searchEntry, i);
                        break;
                    }
                }

            }

        }

        return newEntries;
    }

    public static String getSIPUserName(BasicAttributes attrs){
        SipParser sipParser = null;
        String retVal = null;

        // See if it's a valid SIP URL
        try {
            sipParser = new SipParser(getURL(attrs));
            retVal = sipParser.getDisplayName();
        } catch (Exception e) {
        }

        return retVal;
    }

    public static String getSIPUserId(BasicAttributes attrs){
        SipParser sipParser = null;
        String retVal = null;

        // See if it's a valid SIP URL
        try {
            sipParser = new SipParser(getURL(attrs));
            retVal = sipParser.getUser();
        } catch (Exception e) {
        }

        return retVal;
    }

    public static String getSIPAddress(BasicAttributes attrs){
        SipParser sipParser = null;
        String retVal = null;

        // See if it's a valid SIP URL
        try {
            sipParser = new SipParser(getURL(attrs));
            retVal = sipParser.getHost();
        } catch (Exception e) {
        }

        return retVal;
    }

    public static String getSIPURL(BasicAttributes attrs)
    {
        SipParser sipParser = null;
        String retVal = null;

        // See if it's a valid SIP URL
        try {
            sipParser = new SipParser(getURL(attrs));
            retVal = sipParser.render();
        } catch (Exception e) {
        }

        return retVal;
    }

    public static void main(String argv[])
    {
        // load DLL
        //System.out.println("Current working directory: " + System.getProperty("user.dir") ) ;
        //File fLib = new File(strPath) ;
        //System.out.println("File: " + fLib.getAbsolutePath() + " " + fLib.getName()) ;

        System.load("e:\\pinger\\pingerjni.dll");

        SpeedDialDSP speedDialDSP = SpeedDialDSP.getInstance();

        BasicAttributes attrs = new BasicAttributes();

        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "01");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Joe Shmoe");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "333344444");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) url);

        speedDialDSP.addEntry(attrs);
        attrs = new BasicAttributes();

        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "02");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Jane Doe");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "112233");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "Jane Doe<sip:jane@example.com>");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "03");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Yo mamma");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "04");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "John");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "05");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Bugs");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "06");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Daffy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "07");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Road Runner");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "08");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "The Tick");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "09");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Ed Edd and Eddy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "10");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Rocko");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "11");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Ren and Stimpy");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_ID,            (Object) "12");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Stuff");
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_PHONE_NUMBER,  (Object) "1-234-567-8901");
        //attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_URL,           (Object) "sip:yo@example.com");

        speedDialDSP.addEntry(attrs);

        System.out.println("main:: start dump");
        speedDialDSP.dump();
        System.out.println("main:: end dump");

        System.out.println("Delete Joe");
        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "Joe Shmoe");
        try {
            speedDialDSP.deleteEntry(attrs);
        } catch (Exception e) {
            System.out.println("can't find key");
        }
        speedDialDSP.dump();

        System.out.println("Delete bogus");
        attrs = new BasicAttributes();
        attrs.put(SpeedDialDSP.SPEED_DIAL_SCHEMA_LABEL,         (Object) "bogus");
        try {
            speedDialDSP.deleteEntry(attrs);
        } catch (Exception e) {
            System.out.println("    Bogus delete failed, as expected");
        }
        /*
        System.out.println("Lookup Jane");
        BasicAttributes lookup;
        try {
            lookup = speedDialDSP.lookupName("Jane Doe");
            printAttrs(lookup);
            System.out.println("    SIP stuff:");
            System.out.println("        UserID:   " + getSIPUserId(lookup));
            System.out.println("        UserName: " + getSIPUserName(lookup));
            System.out.println("        URL:      " + getURL(lookup));
            System.out.println("        Address:  " + getSIPAddress(lookup));

        } catch (Exception e) {
            System.out.println("    Bogus delete failed, as expected");
        }
        */

        //System.out.println("Clear everything");
        //speedDialDSP.clear();
        speedDialDSP.dump();
    }

    static void printAttrs(Attributes attrs)
    {
        if (attrs == null) {
            System.out.println("No attributes");
        } else {
            /* print each attribute */
            try {
                for (NamingEnumeration ae = attrs.getAll();
                    ae != null && ae.hasMoreElements();)
                {
                    Attribute attr = (Attribute)ae.next();
                    System.out.println("    attribute: " + attr.getID());

                    /* print each value */
                    for (NamingEnumeration e = attr.getAll();
                        e.hasMoreElements();
                        System.out.println("    value: " + e.nextElement()));
                }
            } catch (NamingException e) {
                SysLog.log(e);
            }
        }
    }

    static void printBindings(NamingEnumeration bl) {
        if (bl == null)
            System.out.println("No items in list");
        else {
            try {
                while (bl.hasMore()) {
                    Binding b = (Binding)bl.next();
                    String bindingName = b.getName();
                    System.out.println(bindingName);
                    printAttrs((Attributes) b.getObject());
                }
            } catch (NamingException e) {
                SysLog.log(e);
            }
        }
    }

}
