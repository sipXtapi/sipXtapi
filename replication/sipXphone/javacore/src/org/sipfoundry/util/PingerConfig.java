/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/PingerConfig.java#3 $
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

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.io.IOException;
import java.io.File;

import org.sipfoundry.util.PingerConfigFileParser;
import org.sipfoundry.sipxphone.sys.util.PingerInfo;

/**
 * This class reads the pinger-config and user-config files and returns them
 * as a Hashtable. If a key appears in both the pinger and user config files,
 * the value in the user-config wins.
 */
public class PingerConfig
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public final static String SIP_FORWARD_UNCONDITIONAL = "SIP_FORWARD_UNCONDITIONAL" ;
    public final static String SIP_FORWARD_ON_BUSY = "SIP_FORWARD_ON_BUSY" ;
    public final static String SIP_FORWARD_ON_NO_ANSWER = "SIP_FORWARD_ON_NO_ANSWER" ;
    public final static String SIP_PROXY_SERVERS = "SIP_PROXY_SERVERS" ;
    public final static String SIP_REGISTRY_SERVERS = "SIP_REGISTRY_SERVERS" ;
    public final static String SIP_DIRECTORY_SERVERS = "SIP_DIRECTORY_SERVERS" ;
    public final static String SIP_TCP_PORT = "SIP_TCP_PORT" ;
    public final static String SIP_UDP_PORT = "SIP_UDP_PORT" ;

    public final static String PHONESET_CALL_WAITING_BEHAVIOR = "PHONESET_CALL_WAITING_BEHAVIOR" ;
    public final static String PHONESET_RINGER = "PHONESET_RINGER" ;
    public final static String PHONESET_BUSY_BEHAVIOR = "PHONESET_BUSY_BEHAVIOR" ;
    public final static String PHONESET_AVAILABLE_BEHAVIOR = "PHONESET_AVAILABLE_BEHAVIOR" ;
    public final static String PHONESET_NO_ANSWER_TIMEOUT = "PHONESET_NO_ANSWER_TIMEOUT" ;
    public final static String PHONESET_FORWARD_UNCONDITIONAL = "PHONESET_FORWARD_UNCONDITIONAL";
    public final static String PHONESET_TIME_DST_RULE = "PHONESET_TIME_DST_RULE" ;
    public final static String PHONESET_TIME_OFFSET = "PHONESET_TIME_OFFSET" ;
    public final static String PHONESET_HTTP_PORT = "PHONESET_HTTP_PORT" ;
    public final static String PHONESET_HTTPS_PORT = "PHONESET_HTTPS_PORT" ;
    public final static String PHONESET_EXTENSION = "PHONESET_EXTENSION" ;
    public final static String PHONESET_EXTERNAL_IP_ADDRESS = "PHONESET_EXTERNAL_IP_ADDRESS" ;
    public final static String PHONESET_HTTP_PROXY_HOST = "PHONESET_HTTP_PROXY_HOST" ;
    public final static String PHONESET_HTTP_PROXY_PORT = "PHONESET_HTTP_PROXY_PORT" ;
    public final static String PHONESET_RTP_PORT_START = "PHONESET_RTP_PORT_START" ;
    public final static String PHONESET_CHECK_SYNC = "PHONESET_CHECK-SYNC" ;
    public final static String PHONESET_VOICEMAIL_RETRIEVE = "PHONESET_VOICEMAIL_RETRIEVE" ;
    public final static String PHONESET_VOICEMAIL_EMPTY_BEHAVIOR = "PHONESET_VOICEMAIL_EMPTY_BEHAVIOR" ;
    public final static String PHONESET_LOGO_URL = "PHONESET_LOGO_URL" ;
    public final static String PHONESET_DND = "PHONESET_DND" ;
    public final static String PHONESET_DND_METHOD = "PHONESET_DND_METHOD" ;
    public final static String PHONESET_TRANSFER_METHOD = "PHONESET_TRANSFER_METHOD" ;
    public final static String PHONESET_VERSIONCHECK_PERIOD = "PHONESET_VERSIONCHECK_PERIOD" ;
    public final static String PHONESET_DIALER_CONTROL_OUTBOUND_LINE = "PHONESET_DIALER_CONTROL_OUTBOUND_LINE" ;
    public final static String PHONESET_DIALER_SHOW_SINGLE_OUTBOUND_LINE = "PHONESET_DIALER_SHOW_SINGLE_OUTBOUND_LINE" ;
    public final static String PHONESET_CLOSE_HUNGUP_FORM_DELAY = "PHONESET_CLOSE_HUNGUP_FORM_DELAY" ;
    public final static String PHONESET_HOME_SCREEN_TITLE = "PHONESET_HOME_SCREEN_TITLE" ;
    public final static String PHONESET_REMOTE_ID_FORMAT = "PHONESET_REMOTE_ID_FORMAT";
    public final static String PHONESET_MWI_LAMPMODE = "PHONESET_MWI_LAMPMODE";
    public final static String PHONESET_HOOKSWITCH_CONTROL = "PHONESET_HOOKSWITCH_CONTROL" ;

    public final static String PHONESET_SHOW_MENU_TAB = "PHONESET_SHOW_MENU_TAB";
    public final static String PHONESET_LOCALE_LANGUAGE = "PHONESET_LOCALE_LANGUAGE";
    public final static String PHONESET_LOCALE_COUNTRY = "PHONESET_LOCALE_COUNTRY";
    public final static String PHONESET_LOCALE_VARIANT = "PHONESET_LOCALE_VARIANT";

    public final static String PHONESET_PHONEBOOK_LDAP_SERVER = "PHONESET_PHONEBOOK_LDAP_SERVER" ;
    public final static String PHONESET_PHONEBOOK_LDAP_PORT = "PHONESET_PHONEBOOK_LDAP_PORT" ;
    public final static String PHONESET_PHONEBOOK_LDAP_ROOT = "PHONESET_PHONEBOOK_LDAP_ROOT" ;
    public final static String PHONESET_PHONEBOOK_LDAP_MAXHITS = "PHONESET_PHONEBOOK_LDAP_MAXHITS" ;
    public final static String PHONESET_PHONEBOOK_LDAP_USERNODE = "PHONESET_PHONEBOOK_LDAP_USERNODE" ;
    public final static String PHONESET_PHONEBOOK_LDAP_USERPASSWORD = "PHONESET_PHONEBOOK_LDAP_USERPASSWORD" ;
    public final static String PHONESET_PHONEBOOK_LDAP_AUTHENTICATE = "PHONESET_PHONEBOOK_LDAP_AUTHENTICATE" ;
    public final static String PHONESET_HOME_SCREEN_CYCLE_SEC = "PHONESET_HOME_SCREEN_CYCLE_SEC" ;
    public final static String PHONESET_PREFS_INACTIVITY_TIMEOUT = "PHONESET_PREFS_INACTIVITY_TIMEOUT" ;
    public final static String PHONESET_RINGDOWN = "PHONESET_RINGDOWN" ;
    public final static String PHONESET_RINGDOWN_ADDRESS = "PHONESET_RINGDOWN_ADDRESS" ;
    public final static String PHONESET_DIAL_BACK_ADDRESS = "PHONESET_DIAL_BACK_ADDRESS" ;
    public final static String PHONESET_MUSIC_ON_HOLD = "PHONESET_MUSIC_ON_HOLD" ;
    public final static String PHONESET_MUSIC_ON_HOLD_SOURCE = "PHONESET_MUSIC_ON_HOLD_SOURCE" ;
    public final static String PHONESET_BRING_TO_FRONT_ON_CALL = "PHONESET_BRING_TO_FRONT_ON_CALL" ;

    public final static String JAVA_OUTPUTSTREAM_REDIRECT = "JAVA_OUTPUTSTREAM_REDIRECT" ;
    public final static String JAVA_FORCE_GC_THRESHOLD_PERCENT   = "JAVA_FORCE_GC_THRESHOLD_PERCENT" ;
    public final static String JAVA_LOW_MEMORY_THRESHOLD_PERCENT = "JAVA_LOW_MEMORY_THRESHOLD_PERCENT" ;

    public final static String JAVA_LOGGER_PORT = "JAVA_LOGGER_PORT" ;
    public final static String JAVA_LOGGER_DEBUG = "JAVA_LOGGER_DEBUG" ;
    public final static String JAVA_CALL_DEBUG = "JAVA_CALL_DEBUG" ;

    public final static String PINGER_CONFIG = "pinger-config";
    public final static String USER_CONFIG = "user-config";
    public final static String LOCAL_CONFIG = "local-config";


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the hash table of configuration data */
    private OrderedHashtable m_htConfigData;

    /** the hash table of group configuration data */
    private Hashtable m_htConfigGroupData;

    /** singleton instance of the pinger config */
    protected static PingerConfig m_reference = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    protected PingerConfig()
    {
    }

    protected void init()
    {
        // This is the Hashtable used to store the current merged config
        m_htConfigData = new OrderedHashtable();
        m_htConfigGroupData = new Hashtable();
        parseConfigFiles() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * get a reference to the pinger config
     */
    public static PingerConfig getInstance()
    {
        if (m_reference == null) {
            m_reference = new PingerConfig() ;
	    m_reference.init();
        }
        return m_reference ;
    }

    /**
     * This method reads the pinger-config and user-config files and returns them
     * as a Hashtable. If a key appears in both the pinger and user config files,
     * the value in the user-config wins.
     */
    public OrderedHashtable getConfig()
    {
        if (m_htConfigData == null)
            parseConfigFiles();

        return m_htConfigData;
    }


    /**
     * Helper method to get a specific value from the pinger-config file
     * @param strKey
     */
    public String getValue(String strKey)
    {
        return (String) m_htConfigData.get( strKey) ;
    }


    /**
     * Helper method to get a Hashtable of data from the pinger-config file
     * for a group key.
     * @param strKey
     */
    public Hashtable getGroupValue(String strKey)
    {
        return (Hashtable) m_htConfigGroupData.get(strKey) ;
    }



    /**
     * Clears the data cache and reparses/loads configuration data
     */
    public void reload()
    {
        parseConfigFiles() ;
    }

    /**
     * Full file system path to a given profile isolating platform differences
     */
    public static String getProfilePath(String profile)
    {
        return new StringBuffer()
            .append(PingerInfo.getInstance().getFlashFileSystemLocation())
            .append(File.separator).append(profile)
            .toString();
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Populate the m_htConfigData data object with the contents of the
     * specified  configuration file.
     */
    private void pullInConfigFile(String configFile)
    {
        String fullConfigFileName = getProfilePath(configFile);

        try 
        {
            OrderedHashtable temp = PingerConfigFileParser.getConfigFile(fullConfigFileName);
            Enumeration keysEnum   = temp.getOrderedKeys();
            while (keysEnum.hasMoreElements())
        {
            String key   = (String) keysEnum.nextElement();
                String value = (String) temp.get(key);
            m_htConfigData.put(key, value);
            PingerConfigFileParser.handleGroupData( m_htConfigGroupData, key, value);
        }
    }
        catch (IOException e) 
        {
            System.out.println("Error getting " + fullConfigFileName);
        }

    }

    /**
     * Parse the user-config and pinger-config files and stuff all of the
     * contents into the m_htConfigData hash table.
     */
    private void parseConfigFiles()
    {
        // Wipe out the old contents
        m_htConfigData = new OrderedHashtable();

        // Populate with our two configuration files.  Order is important here.
        pullInConfigFile(PINGER_CONFIG);
        pullInConfigFile(USER_CONFIG);
        pullInConfigFile(LOCAL_CONFIG);
    }


}
