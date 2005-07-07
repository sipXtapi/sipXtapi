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


package org.sipfoundry.sipxphone.sys.util ;

import java.util.Properties ;
import java.util.StringTokenizer ;
import java.util.Vector ;
import java.util.Locale;
import java.util.Enumeration ;
import java.io.File;

import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.sipxphone.sys.SystemDefaults;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.Repository ;


/**
 * Utility class to provide information regarding state of the softphone
 */
public class PingerInfo
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static final String DEFAULT_RINGTONE_FILE_NAME = "ringTone.raw";

    /** Reference to singleton PingerInfo instance */
    protected static PingerInfo m_pingerReference = null ;

    public static final int PBT_UNKNOWN = -1 ;
    public static final int PBT_BRUTUS = 0 ;
    public static final int PBT_TCAS1 = 1 ;
    public static final int PBT_TCAS2 = 2 ;
    public static final int PBT_TCAS3 = 3 ;
    public static final int PBT_TCAS4 = 4 ;
    public static final int PBT_TCAS5 = 5 ;
    public static final int PBT_TCAS6 = 6 ;
    public static final int PBT_TCAS7 = 7 ;

    private static boolean s_bUseTimeSinceBoot = true ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor
     * made private so that no other class makes a new instance of it
     * anyone who wants an instance of PingerInfo should call
     * PingerInfo.getInstance()  method
     */
    private PingerInfo()
    {

    }



//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////

    /**
     * get a singleton reference to the PingerInfo object.
     *
     * @return PingerInfo reference
     */
    public static PingerInfo getInstance()
    {
        // Create if needed
        if (m_pingerReference == null) {
            m_pingerReference = (PingerInfo) new PingerInfo() ;
        }

        return m_pingerReference ;
    }

    /**
     * What is the filename of the default ring file?
     * @return the full path of the default ring file
     */
    public String getDefaultRingFileName(){
        return  ( getFlashFileSystemLocation()
                  + File.separator
                  + DEFAULT_RINGTONE_FILE_NAME );
    }

    /**
     * Does the default ring file exist?
     */
    public boolean doesDefaultRingFileExists(){
        boolean bExists = false;
        File fileDefaultRing = new File( getDefaultRingFileName() );
        if( fileDefaultRing.exists() )
            bExists = true;
        return bExists;
    }

    /**
     * what is the location of the flash file system ?
     */
    public String getFlashFileSystemLocation()
    {
        return JNI_getFlashFileSystemLocation() ;
    }

    /**
     * What version of the kernel are we using?
     */
    public String getKernelVersion()
    {
        return JNI_getVersion() ;
    }


    /**
     * When was the kernel built?
     */
    public String getKernelBuildDate()
    {
        return JNI_getKernelBuildDate() ;
    }


    /**
     * What was the comment included as part of the build
     */
    public String getKernelBuildComment()
    {
        return JNI_getKernelBuildComment() ;
    }


    /**
     * What is the platform type?
     */
    public String getPlatformType()
    {
        return JNI_getPlatformType() ;
    }


    /**
     * Get the board type for this platform
     */
    public int getBoardType()
    {
        return JNI_getBoardType() ;
    }

    /**
     * returns the number of Call threads currently being tracked by the lower layer
     * calls are leaking when the upper layer believes there are no calls
     * but this function returns > 0
     */
    public int getCallTaskCount()
    {
        return JNI_getCallTaskCount() ;
    }

    /**
     * Gets the uptime (time since boot) of this phone and returns in a
     * formated string (x day(s) hh:mm:ss).
     */
    public String getUptime()
    {
        return JNI_getUptime() ;
    }


    /**
     * Get the current time in MS.
     */
    public long getTimeInMS()
    {
        long lTime = 0 ;

        if (s_bUseTimeSinceBoot)
        {
            try
            {
                lTime = JNI_getTimeInMS() ;
            }
            catch (UnsatisfiedLinkError error)
            {
                lTime = System.currentTimeMillis() ;
                s_bUseTimeSinceBoot = false ;
            }
        }
        else
        {
            lTime = System.currentTimeMillis() ;
        }

        return lTime ;
    }

/////////////////////////////////////////////////////////////////////////////
// Deployment Server & Ixpressa Routines
////

    /**
     * Return physical id of phone
     */
    public String getMacAddress()
    {
        return JNI_getMacAddress() ;
    }


    /**
     * Get digest
     */
    public String getDigest(String strSecret)
    {
        return JNI_digest(strSecret) ;
    }


    /**
     * Get LogicalIdURL
     */
    public String setupGetLogicalIdURL(String strAdminDomain, String strAdminId, String strAdminPwd, String strSecret, String strLogicalId)
    {
        if (strLogicalId == "")
            return JNI_setupGetLogicalIdUrl2(strAdminDomain, strAdminId, strAdminPwd, strSecret) ;
        else
            return JNI_setupGetLogicalIdUrl(strAdminDomain, strAdminId, strAdminPwd, strSecret, strLogicalId) ;

    }


    /**
     * Get ConfigURL
     */
    public String setupGetConfigURL(String strAdminDomain, String strLogicalId, String strSecret )
    {
        return JNI_setupGetConfigUrl(strAdminDomain, strLogicalId, strSecret ) ;
    }


    /**
     * Get expiry date of key-config (for instant xpressa)
     */
    public String getExpireDate()
    {
        return JNI_getExpireDate() ;
    }


    /**
     * Get admin domain of key-config (for instant xpressa)
     */
    public String getAdminDomain()
    {
        return JNI_getAdminDomain() ;
    }

    /**
     * Get host name of key-config (for instant xpressa)
     */
    public String getHostName()
    {
        return JNI_getHostName() ;
    }

    /**
     * Get admin domain of key-config (for instant xpressa)
     */
    public String getSerialNum()
    {
        return JNI_getSerialNum() ;
    }

    /**
     * Get key of key-config (for instant xpressa)
     */
    public String getKey()
    {
        return JNI_getKey() ;
    }

    /**
     * Return whether license for this product is valid
     */
    public int isLicenseValid()
    {
        return JNI_isLicenseValid() ;
    }

    /**
     * Return error code if isLicenseValid returns 0
     */
    public int getLicenseFailureCode()
    {
        return JNI_getLicenseFailureCode() ;
    }

    /**
     * gets the locale for the system.
     * Locale can be defined by these parameters:<br>
     * PHONESET_LOCALE_LANGUAGE    ( defaults to "en" )<br>
     * PHONESET_LOCALE_COUNTRY     ( defaults to "US" )<br>
     * PHONESET_LOCALE_VARIANT<br>
     * A Locale is constructed using the language code, country code and
     * the variant if available.
     */
    public Locale getLocale()
    {
        Locale retLocale;
        String strLocaleLang =
            PingerConfig.getInstance().getValue(PingerConfig.PHONESET_LOCALE_LANGUAGE) ;
        String strLocaleCountry =
            PingerConfig.getInstance().getValue(PingerConfig.PHONESET_LOCALE_COUNTRY) ;
        if( strLocaleLang == null )
            strLocaleLang = SystemDefaults.LOCALE_LANGUAGE;
        if( strLocaleCountry == null )
            strLocaleCountry = SystemDefaults.LOCALE_COUNTRY;
        String strLocaleVariant =
            PingerConfig.getInstance().getValue(PingerConfig.PHONESET_LOCALE_VARIANT) ;
        if( ! TextUtils.isNullOrSpace(strLocaleVariant) )
        {
            retLocale = new Locale( strLocaleLang, strLocaleCountry, strLocaleVariant );
        }else
        {
            retLocale = new Locale( strLocaleLang, strLocaleCountry );
        }
        return retLocale;
    }

    /**
     * Return whether key-config file is valid (for instant xpressa)
     */
    public int getAppLimit()
    {
        return JNI_getAppLimit() ;
    }
    /**
     * Return the license
     */
    public Vector getLicenseFeatures()
    {
        Vector v = null;
        String features = JNI_getLicenseFeatures() ;

        if (features != null)
        {
            v = new Vector();
             StringTokenizer st = new StringTokenizer(features);

            while (st.hasMoreTokens()) {
                v.addElement(st.nextToken());
            }

        }

        return v;
    }

    /**
     * Return the license key type
     */
    public String getKeyConfig()
    {
        return JNI_getLicenseType() ;
    }

    public void unregisterAllLines()
    {
        JNI_unregisterAllLines();
    }

    public void unSubscribeAll()
    {
        JNI_unSubscribeAll();
    }

    /**
     * Reboot the phone
     */
    public void reboot()
    {
        // Force Garbage Collection before requesting a restart
        Runtime.getRuntime().gc();
        Runtime.getRuntime().runFinalization();

        Shell.getInstance().displayStatus("Restarting...") ;

        unregisterAllLines() ;
        unSubscribeAll() ;

        // Flush the Repository...
        Repository.getInstance().flush() ;
        try
        {
            Thread.sleep(500) ;
        }
        catch(Exception e)
        {
                System.out.println(e);
        }

        if (!(System.getProperties().get(PingerApp.TESTBED_WORLD)=="true")) {
            // restart Pinger
            JNI_stopPinger() ;
        } else {
            try
            {
                Thread.sleep(500);
                System.exit(2);
            }
            catch(Exception e)
            {
                System.out.println(e);
            }
        }
    }

    /**
     * Reboot the phone
     */
    public void rebootNoFlush()
    {
        Shell.getInstance().displayStatus("Restarting...") ;

        unregisterAllLines() ;
        unSubscribeAll() ;


        if (!(System.getProperties().get(PingerApp.TESTBED_WORLD)=="true")) {
            // restart Pinger
            JNI_stopPinger() ;
        } else {
            try
            {
                Thread.sleep(500);
                System.exit(2);
            }
            catch(Exception e)
            {
                System.out.println(e);
            }
        }
    }

    /**
     * Reset the phone.  Please invoke this method with caution!  It will
     * remove all of the user settings and installed applications (equivalent
     * to hitting **9 in the tcas2 days).
     */
    public void restoreFactoryDefaults()
    {
        JNI_restoreFactoryDefaults() ;
    }

/////////////////////////////////////////////////////////////////////////////
// xpressa configuration routines
////

    /**
     * Get SIP version phone is using
     */
    public String getSipVersion()
    {
            return JNI_getSipVersion() ;
    }

    /**
     * What is the max number of remote connections allowed in the system.
     */
    public int getMaxSimultaneousConnections()
    {
        int iMaxConnections = 4 ;

        try {
            String strMaxConnections = PingerConfig.getInstance().getValue("PHONESET_MAX_CONNECTIONS") ;
            if (strMaxConnections != null)
                iMaxConnections = Integer.parseInt(strMaxConnections) ;
        } catch (Exception e) {
            System.out.println("Unable to parse PHONESET_MAX_CONNECTIONS") ;
            SysLog.log(e) ;
        }
        return iMaxConnections ;
    }

   /**
    * What is the number of remote connections currently allocated in the
    * system.
    */
    public int getSimultaneousConnections()
    {
        PCallManager callManager = Shell.getCallManager() ;

        int iConnections = 0 ;

        // Traverse all of the calls and
        PCall calls[] = callManager.getCalls() ;
        for (int i=0; i<calls.length; i++) {
            PAddress participants[] = calls[i].getParticipants() ;
            iConnections += participants.length ;
        }
        return iConnections ;
    }


    /**
     * Determine if we are running under Insigna's EVM
     */
    public boolean isEVM()
    {
        boolean bRC = false ;

        Properties props = System.getProperties() ;
        if (System.getProperty("com.windriver.vm.name", "unknown").equalsIgnoreCase("evm"))
        {
            bRC = true ;
        }

        return bRC ;
    }
    
    
    /**
     * Is the SIP Log presently enabled on this phone?
     *  
     * @return true if enabled otherwise false
     */
    public boolean isSipLogEnabled()
    {
    	return JNI_isSipLogEnabled() ;
    }
    
    
    /**
     * Store the SIP Log to the specified file spec.   
     * 
     * @param strFilespec full path to the output file
     * @return true if successful, otherwise false.
     */
    public boolean saveSipLog(String strFilespec)
    {			
		return JNI_saveSipLog(strFilespec) ;
    }
    
    /**
     * Enables or disables the SIP Log.
     * 
     * @param bEnable true to enable or false to disable
     */
    public void enableSipLog(boolean bEnable)
    {
    	JNI_enableSipLog(bEnable) ;
    }


/////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * String tokenize the specified string by the specified delimiter list
     * and return each individual token as a separate vector element.
     */
    protected Vector vectorizeString(String strSource, String strDelimiters)
    {
        Vector vList = new Vector() ;

        if (strSource != null) {
            StringTokenizer st = new StringTokenizer(strSource, strDelimiters) ;
            while (st.hasMoreTokens()) {
                vList.addElement(st.nextToken()) ;
            }
        }
        return vList ;
    }


    /**
     * Converts the specified vector into a string where each element is
     * delimited by the specified strDelimiter.  Blank elements are ignored.
     */
    protected String stringizeVector(Vector v, String strDelimiter)
    {
        StringBuffer strList = new StringBuffer("") ;

        if (v != null) {
            int iCount = v.size() ;
            for (int i=0; i<iCount; i++) {
                String strElement = null ;
                Object objElement = v.elementAt(i) ;
                if (objElement != null)
                    strElement = objElement.toString() ;

                if ((strElement != null) && (strElement.length() > 0)) {
                    strList.append(strElement) ;
                    if ((i+1) < iCount)
                        strList.append(strDelimiter) ;
                }
            }
        }
        return strList.toString() ;
    }


/////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native String JNI_getFlashFileSystemLocation() ;
    protected static native String JNI_getVersion() ;
    protected static native String JNI_getKernelBuildDate() ;
    protected static native String JNI_getKernelBuildComment() ;
    protected static native String JNI_getPlatformType() ;

    protected static native int    JNI_getCallTaskCount() ;
    protected static native String JNI_getUptime() ;
    protected static native long   JNI_getTimeInMS() ;

    // for deployment server
    protected static native String JNI_getMacAddress() ;
    protected static native String JNI_digest(String strSecret) ;
    protected static native String JNI_setupGetLogicalIdUrl2(String strAdminDomain, String strAdminId, String strAdminPwd, String strSecret) ;
    protected static native String JNI_setupGetLogicalIdUrl(String strAdminDomain, String strAdminId, String strAdminPwd, String strSecret, String strLogicalId) ;
    protected static native String JNI_setupGetConfigUrl(String strAdminDomain, String strLogicalId, String strSecret ) ;

    // for instant xpressa keyfile
    protected static native int     JNI_isLicenseValid() ;
    protected static native int     JNI_getLicenseFailureCode() ;
    protected static native String  JNI_getExpireDate() ;
    protected static native String  JNI_getAdminDomain() ;
    protected static native String  JNI_getHostName() ;
    protected static native String  JNI_getSerialNum() ;
    protected static native String  JNI_getKey() ;
    protected static native int     JNI_getAppLimit() ;
    protected static native String  JNI_getLicenseFeatures() ;
    protected static native String  JNI_getLicenseType() ;

    // to reboot / reset
    protected static native void    JNI_startPinger() ;
    protected static native void    JNI_stopPinger() ;
    protected static native void    JNI_restoreFactoryDefaults() ;
    protected static native void    JNI_unSubscribeAll() ;
    protected static native void    JNI_unregisterAllLines() ;

    // xpressa configuration
    protected static native int JNI_getBoardType() ;

    protected static native void JNI_enableSipLog(boolean bEnable) ;
    protected static native boolean JNI_isSipLogEnabled() ;
    protected static native boolean JNI_saveSipLog(String strFilespec);
    
    // other useful information
    protected static native String  JNI_getSipVersion() ;
}
