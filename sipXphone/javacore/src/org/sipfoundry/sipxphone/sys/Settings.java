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


package org.sipfoundry.sipxphone.sys ;

import java.util.* ;
import java.io.* ;

import org.sipfoundry.sipxphone.sys.util.PingerInfo ;
import org.sipfoundry.sipxphone.sys.XpressaSettings ;
import org.sipfoundry.sipxphone.sys.Shell ;

/**
 * Settings is a simple utility class that allows code to quickly get a system
 * settings.  This contains stuff that users generally will not need to touch
 * (e.g. tooltip timeouts, Phonebook application, etc), however, should be
 * easilychanged.
 * <br><br>
 * Static methods getInt and getString should be used to get integer and
 * string settings.  These calls also require a default value in case the
 * file is not found or specific key is not found.  This is important and
 * developers should not count on this file being present.  A "Reset to
 * factory defaults" setting could blow away the settings file.
 * <br><br>
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class Settings
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** file our settings are read from */
    protected static final String PINGER_SETTINGS_RESFILE_BASE = "settings" ;
    protected static final String PINGER_SETTINGS_RESFILE_EXT  = ".properties" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** singleton reference to ourself */
    protected static Settings m_instance = null ;

    /** resource bundle containing all of our settings */
    // protected PropertyResourceBundle  m_settings ;

    protected Hashtable m_htSettings ;

    /** Settings can be overridden if desired */
    protected Hashtable m_htOverrides ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor, use getInstance()
     */
    protected Settings()
    {
        m_htSettings = new Hashtable() ;
        m_htOverrides = new Hashtable() ;

        // Load Base Settings
        String strResourceFile = PINGER_SETTINGS_RESFILE_BASE +
                PINGER_SETTINGS_RESFILE_EXT ;
        try
        {
            PropertyResourceBundle bundle = new PropertyResourceBundle(ClassLoader.getSystemResourceAsStream(strResourceFile)) ;
            populateHashtableFromResourceBundle(m_htSettings, bundle) ;
        }
        catch (Exception e)
        {
            System.out.println("Settings: Cannot load resource bundle: " + strResourceFile) ;
        }

        // Load Product Settings
        strResourceFile = PINGER_SETTINGS_RESFILE_BASE + "-" +
                PingerInfo.getInstance().getPlatformType() +
                PINGER_SETTINGS_RESFILE_EXT ;

        try
        {

            PropertyResourceBundle bundle = new PropertyResourceBundle(ClassLoader.getSystemResourceAsStream(strResourceFile)) ;
            populateHashtableFromResourceBundle(m_htSettings, bundle) ;
        }
        catch (Exception e)
        {
            System.out.println("Settings: Cannot load resource bundle: " + strResourceFile) ;
        }
    }



    /**
     * get an singleton instance
     */
    public static Settings getInstance()
    {
        // Only create if needed.
        if (m_instance == null) {
            m_instance = new Settings() ;
        }

        return m_instance ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get a string setting
     *
     * @param strKey key you want to lookup
     * @param strDefault default value to used if not found
     */
    public static String getString(String strKey, String strDefault)
    {
        String                  strRC = null ;
        Hashtable               htSettings = getInstance().getSettings() ;
        Hashtable               htOverrides = getInstance().getOverrides() ;

        // On any error, just return the default
        strRC = (String) htOverrides.get(strKey) ;
        if (strRC == null)
            strRC = (String) htSettings.get(strKey) ;
        if (strRC == null)
            strRC = strDefault ;

        return strRC ;
    }


    /**
     * Get an integer setting
     *
     * @param strKey key you want to lookup
     * @param iDefault default value to used if not found or bad data
     */
    public static int getInt(String strKey, int iDefault)
    {
        int                     iRC = 0 ;
        Hashtable               htSettings = getInstance().getSettings() ;
        Hashtable               htOverrides = getInstance().getOverrides() ;

        // On any error, just return the default
        try {
            String strValue = (String) htOverrides.get(strKey) ;
            if (strValue == null)
                strValue = (String) htSettings.get(strKey) ;
            if (strValue == null)
                iRC = iDefault ;
            else
                iRC = Integer.parseInt(strValue) ;
        } catch (NumberFormatException nfe) {
            System.out.println("Settings: Invalid int setting: " + nfe) ;
            iRC = iDefault ;
        }

        return iRC ;
    }


    /**
     * Set a dynamic override for the compiled-in settings
     */
    public static void setOverride(String strKey, String strValue)
    {
        Hashtable htOverrides = getInstance().getOverrides() ;

        htOverrides.put(strKey, strValue) ;
    }


    /**
     * Clear a dynamic override for the compiled-in settings
     */
    public static void clearOverride(String strKey)
    {
        Hashtable htOverrides = getInstance().getOverrides() ;

        htOverrides.remove(strKey) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * return the settings hash
     */
    protected Hashtable getSettings()
    {
        return m_htSettings ;
    }


    /**
     * return the overrides hash
     */
    protected Hashtable getOverrides()
    {
        return m_htOverrides ;
    }


    /**
     * Fills the hashtable with data from the Property Resource Bundle
     */
    protected void populateHashtableFromResourceBundle(Hashtable ht, PropertyResourceBundle prb)
    {
        Enumeration keys = prb.getKeys() ;
        while (keys.hasMoreElements())
        {
            String strKey = (String) keys.nextElement() ;
            String strValue = prb.getString(strKey) ;

            if (strValue != null)
            {
                ht.put(strKey, strValue) ;
            }
        }
    }
}
