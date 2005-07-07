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
import java.io.File;
import java.io.InputStream;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp ;
import javax.telephony.phone.*;
/**
 * This class file allows developers to query for network, phone, and
 * SIP specific settings.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class XpressaSettings
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** xpressa Product Identifer: This product code identifies the physical line
        of xpressa phone devices */
    public static final int PRODUCT_XPRESSA = 1 ;
    /** instant xpressa Product Identifer: This product code identifies the
        emulation kits/instant xpressa softphone*/
    public static final int PRODUCT_INSTANTXPRESSA = 2 ;

    /** default extension of a phone is 4444 **/
    private static final String PHONESET_DEFAULT_EXTENSION = "4444";

    /** the xpressa version property file to pull version information out of */
    private String XPRESSA_VERSION_RESFILE = "xpressa-version.properties" ;

    /** the xpressa resources version property file to pull version information out of */
    private String XPRESSA_RESOURCES_VERSION_RESFILE = "resources-version.properties" ;

    /** the xpressa resources version property file to pull version information out of */
    private String XPRESSA_RESOURCES_JAR_FILE = "resources.jar" ;
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /**
     * vector that stores the callerid  patterns specified for displaying
     * caller id information.
     */
    private Vector m_vCallerIDPatterns;
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Construction is limited to the org.sipfoundry.sipxphone.sys package.
     *
     * @see org.sipfoundry.sipxphone.sys
     */
    protected XpressaSettings()
    {
    }

//////////////////////////////////////////////////////////////////////////////
// Public Method
////

    /**
     * Return the extension value for this xpressa phone.
     *
     * @return The extension of this phone.
     *
     * @deprecated as of 1.3
     */
    public String getExtension()
    {
        String strRC = null ;

        strRC = (String) PingerConfig.getInstance().getValue(PingerConfig.PHONESET_EXTENSION) ;
        if( isNullOrEmpty(strRC))
            strRC = PHONESET_DEFAULT_EXTENSION ;

        return convertEmptyStringToNull(strRC) ;
    }


    /**
     * Get the Ethernet MAC address of this xpressa phone.
     *
     * @return The Ethernet MAC address of this phone.
     */
    public String getMACAddress()
    {
        return PingerInfo.getInstance().getMacAddress() ;
    }


    /**
     * Return the Serial Number of this instant xpressa softphone. If used for an
     * xpressa phone appliance, returns the Ethernet MAC address (like
     * <i>getMACAddress</i>).
     *
     *
     * @return The serial number of an instant xpresssa softphone or the Ethernet
     *         MAC address of an xpressa phone.
     */
    public String getSerialNumber()
    {
        return PingerInfo.getInstance().getSerialNum() ;
    }


    /**
     * pouplates an internal vector with the patterns specified for displaying
     * caller id information.
     * More on this:<br>
     * The parameter PHONESET_REMOTE_ID_FORMAT controls the default information
     *  displayed to the local users for remote caller-id and called-id.
     *  This parameter is also consulted when displaying held call information,
     *  conference party information, and call log information.
     *
     *  The values for the caller-id are case insensitive.  The default
     *  value is:
     *      {name}\n{sipurl}
     *  The values for the caller-id are case insensitive.
     * Options include:
     * {displayname} : Remote party's display name.  If no display name is
     *                 provided, then "Unknown" is displayed.
     * {sipurl}      : SIP URL in the format {username}@{hostname}
     * {username}    : Remote party's user name.
     * {hostname}    : Remote party's hostname.
     * {name}        : Remote party's displayname or username.  The
                       displayname is used if available, otherwise the
                       username is presented. If the username is not available,
                       "Unknown" is used.
     *
     *  A "\n" is used to mark the difference between the first line of text
     *  (larger font) and the second line of text (smaller font).  Only a
     *  single "\n" is expected within the string.  If more are found, they
     *  are ignored.
     *  In the conference, held call and call log forms, only the first line
     *  of text is displayed (data before first "\n").
     *
     *  NOTE: If the display name is "anonymous" (case insensitive), then
     *         {sipurl}, {username}, and {hostname} resolve to empty strings.
     * @deprecated DO NOT EXPOSE
     */
    public Vector getCallerIDPatterns()
    {
        if( m_vCallerIDPatterns == null )
        {
            m_vCallerIDPatterns = new Vector();
            String strRemoteCallerIDFormat = PingerConfig.getInstance()
                .getValue(PingerConfig.PHONESET_REMOTE_ID_FORMAT) ;
            int iIndex = -1;

            //there is something weird about reading a line with "\n" in it by using
            //readLine method of BufferedReader. It somehow escapes it when it returns
            //it to you.
            String strDelimiter = "\\n";
            if( strRemoteCallerIDFormat == null )
            {
                strRemoteCallerIDFormat = SystemDefaults.CALLERID_PATTERN;
                //if its from the system, then the delimiter is the usual "\n"
                strDelimiter = "\n";
            }

            if( strRemoteCallerIDFormat != null )
            {
                //continue to support old values
                if( (iIndex=strRemoteCallerIDFormat.indexOf(strDelimiter)) != -1 )
                {
                    String strToken = strRemoteCallerIDFormat.substring(0, iIndex );
                    if( strToken != null )
                    {
                        m_vCallerIDPatterns.addElement( strToken );
                    }

                    if( strRemoteCallerIDFormat.length() >= iIndex+2 )
                    {
                        strRemoteCallerIDFormat = strRemoteCallerIDFormat.substring( iIndex+2 );
                    }
                }
                m_vCallerIDPatterns.addElement( strRemoteCallerIDFormat );
            }
        }
        return m_vCallerIDPatterns;
    }


    /**
     * Get the HTTP proxy host address.
     *
     * @return The address of the HTTP proxy.
     *
     * @see org.sipfoundry.sipxphone.sys.XpressaSettings#getHTTPProxyHostPort()
     */
    public String getHTTPProxyHost()
    {
        String strRC = null ;

        strRC = (String) PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTP_PROXY_HOST) ;

        return convertEmptyStringToNull(strRC) ;
    }


    /**
     * Get the HTTP proxy port.
     *
     * @return The HTTP proxy port or -1 if the port is not defined.
     *
     * @see org.sipfoundry.sipxphone.sys.XpressaSettings#getHTTPProxyHost()
     */
    public int getHTTPProxyHostPort()
    {
        int iRC = -1 ;

        String strPort = (String) PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTP_PROXY_PORT) ;
        if ((strPort != null) && (strPort.length() > 0)) {
            try {
                iRC = Integer.parseInt(strPort) ;
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid port address specified as the HTTP proxy port") ;
                SysLog.log(nfe);
            }
        }

        return iRC ;
    }

    /**
     *get Message Waiting Indicator Lamp Mode
     */
    public int geMWILampMode()
    {
        int iRC = PhoneLamp.LAMPMODE_STEADY ;
        String strLampMode =
            PingerConfig.getInstance().getValue(PingerConfig.PHONESET_MWI_LAMPMODE) ;
        if (strLampMode != null)
        {
            if( strLampMode.equals("BROKENFLUTTER") )
                iRC = PhoneLamp.LAMPMODE_BROKENFLUTTER;
            else if( strLampMode.equals("FLASH") )
                iRC = PhoneLamp.LAMPMODE_FLASH;
            else if( strLampMode.equals("FLUTTER") )
                iRC = PhoneLamp.LAMPMODE_FLUTTER;
            else if( strLampMode.equals("OFF") )
                iRC = PhoneLamp.LAMPMODE_OFF;
            else if( strLampMode.equals("STEADY") )
                iRC = PhoneLamp.LAMPMODE_STEADY;
            else if( strLampMode.equals("WINK") )
                iRC = PhoneLamp.LAMPMODE_WINK;
        }
        return iRC ;
    }

    /**
     * Return the SIP Proxy Server host address, if set for this xpressa phone.
     *
     * @return The SIP Proxy Server host address.
     *
     */
    public String getSIPProxyServerHost()
    {
        String strRC = null ;

        strRC = (String) PingerConfig.getInstance().getValue(PingerConfig.SIP_PROXY_SERVERS) ;

        return convertEmptyStringToNull(strRC) ;
    }


    /**
     * Get the SIP Directory Server host address, if set for the xpressa phone.
     *
     * @return The SIP Directory Server host address.
     */
    public String getSIPDirectoryServerHost()
    {
        String strRC = null ;

        strRC = (String) PingerConfig.getInstance().getValue(PingerConfig.SIP_DIRECTORY_SERVERS) ;

        return convertEmptyStringToNull(strRC) ;
    }

    /**
     * Return the SIP Registry Server host address, if set for this xpressa phone.
     *
     * @return The SIP Registry Server host address.
     *
     * @deprecated as of 1.3
     */
    public String getSIPRegistryServerHost()
    {
        String strRC = null ;

        strRC = (String) PingerConfig.getInstance().getValue(PingerConfig.SIP_REGISTRY_SERVERS) ;

        return strRC ;
    }


    /**
     * Get the SIP TCP port for this xpressa phone's user agent.
     *
     * @return The SIP TCP port for this phone's user agent, or -1 if undefined.
     *
     * @see org.sipfoundry.sipxphone.sys.XpressaSettings#getSIPUDPPort()
     */
    public int getSIPTCPPort()
    {
        int iRC = -1 ;

        String strPort = (String) PingerConfig.getInstance().getValue(PingerConfig.SIP_TCP_PORT) ;
        if ((strPort != null) && (strPort.length() > 0)) {
            try {
                iRC = Integer.parseInt(strPort) ;
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid port address specified as the SIP TCP port") ;
                SysLog.log(nfe);
            }
        }
        return iRC ;
    }


    /**
     * Return the SIP UDP port for this xpressa phone's user agent.
     *
     * @return The SIP UDP port for this phone's user agent, or -1 if undefined.
     *
     * @see org.sipfoundry.sipxphone.sys.XpressaSettings#getSIPTCPPort()
     */
    public int getSIPUDPPort()
    {
        int iRC = -1 ;

        String strPort = (String) PingerConfig.getInstance().getValue(PingerConfig.SIP_TCP_PORT) ;
        if ((strPort != null) && (strPort.length() > 0)) {
            try {
                iRC = Integer.parseInt(strPort) ;
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid port address specified as the SIP UDP port") ;
                SysLog.log(nfe);
            }
        }

        return iRC ;
    }


    /**
     * Get the starting RTP port used by this xpressa phone.
     *
     * @return The starting RTP port used by this phone.
     */
    public int getRTPStartingPort()
    {
        int iRC = -1 ;

        String strPort = (String) PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RTP_PORT_START) ;
        if ((strPort != null) && (strPort.length() > 0)) {
            try {
                iRC = Integer.parseInt(strPort) ;
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid port address specified as the RTP starting port") ;
                SysLog.log(nfe);
            }
        }

        return iRC ;
    }


    /**
     * Return the HTTP web server port used by this xpressa phone.
     *
     * @return The HTTP web server port or -1 if the web server is not enabled.
     */
    public int getHTTPPort()
    {
        int iRC = -1 ;

        String strPort = (String) PingerConfig.getInstance().getValue(PingerConfig.PHONESET_HTTP_PORT) ;
        if ((strPort != null) && (strPort.length() > 0)) {
            try {
                iRC = Integer.parseInt(strPort) ;
            } catch (NumberFormatException nfe) {
                System.out.println("Invalid port address specified as the HTTP port") ;
                SysLog.log(nfe);
            }
        }

        return iRC ;
    }


    /**
     * Get the version of the core applications installed on this xpressa phone.
     *
     * @return The version of the core applications.
     */
    public String getVersion()
    {
        return getXpressaSettingValue("version");
    }

    /**
     * Get the version of the resources installed on this xpressa phone.
     * The XPRESSA_RESOURCES_VERSION_RESFILE  is expected to be in resources.jar .
     * If properties file is not found, but the jar file exists, returns "Unknown".
     * If the jar file does not exist, returns null.
     * @return The version of the resources.
     */
    public String getResourcesVersion()
    {
        String strRet = null ;

        try
        {
            String strResourcesVersionFullFileName =
                PingerInfo.getInstance().getFlashFileSystemLocation() + File.separator
                + XPRESSA_RESOURCES_JAR_FILE;
            File file = new File( strResourcesVersionFullFileName );
            if( file.exists() )
            {
                InputStream stream =
                    ClassLoader.getSystemResourceAsStream(XPRESSA_RESOURCES_VERSION_RESFILE);
                if( stream != null )
                {
                    PropertyResourceBundle versionBundle = new PropertyResourceBundle(stream);
                    strRet = versionBundle.getString("version");
                }
                if( strRet == null )
                    strRet = "Unknown";
            }
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }
        return strRet;
    }



   /**
    * Return the build number of the core applications installed on this
    * xpressa phone. The build number is reset to zero after any change to
    * the version string.
    *
    * @return The build number of the core applications.
    */
    public int getBuildNumber()
    {
        int iBuildNumber = 0 ;
        String strBuildNumber =  getXpressaSettingValue("buildNumber");
        try{
            if ((strBuildNumber != null) && (strBuildNumber.length() > 0))
            {
                iBuildNumber = Integer.parseInt(strBuildNumber) ;
            }
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }
        return iBuildNumber ;
    }


       /**
        * Return build-specific comments defined for a particular release of
        * the core xpressa phone applications. Build comments are typically
        * used to identify prereleases, beta versions, etc.
        *
        *
        * @return The build-specific comment or null if not defined.
        */
    public String getBuildComment()
    {
        return getXpressaSettingValue("optionalComment");
    }


       /**
        * Return the date and time that the core xpressa phone applications
        * were built.
        *
        * @return A string containing the time and date of the build or null if
        * not definied.
        */
    public String getBuildDate()
    {
        return getXpressaSettingValue("built");
    }


    /**
     * Return the version of the kernel file installed on this xpressa phone.
     *
     * @return The version of the kernel.
     */
    public String getKernelVersion()
    {
        return PingerInfo.getInstance().getKernelVersion() ;
    }


    /**
     * Get the product identifier for this phone.  See the description for the
     * PRODUCT_XPRESSA and PRODUCT_INSTANTXPRESSA constants for more information.
     *
     * @return A code that identifies the Pingtel phone product: xpressa
     * phone devices return 1; instant xpressa softphones return 2.
     *
     */
    public int getProductID()
    {
        if (PingerApp.isTestbedWorld()) {
            return PRODUCT_INSTANTXPRESSA ;
        } else {
            return PRODUCT_XPRESSA ;
        }
    }

    /**
     * get the delay in milliseconds for "Caller Hung Up" or "Call Redirected"
     * or "Call Disconnected" form to close. The default is 10 seconds( 10000 ms).
     * The max delay is 24 hrs ie 86400*1000 ms.
     * The least delay is 0 ms.
     */
    public static int getHungUpFormCloseDelay()
    {
        int iDelay = 10;
        String strMissedCallFormCloseDelay =
            PingerConfig.getInstance()
                .getValue(PingerConfig.PHONESET_CLOSE_HUNGUP_FORM_DELAY ) ;
        if(  strMissedCallFormCloseDelay != null )
        {
            try
            {
                iDelay = Integer.parseInt( strMissedCallFormCloseDelay );
            }
            catch( NumberFormatException e)
            {
                SysLog.log("PHONESET_CLOSE_HUNGUP_FORM_DELAY  parameter does not take a non-numeric value");
                SysLog.log(e);
            }
            //24 hrs
            if( iDelay > 86400 )
            {
                SysLog.log("PHONESET_CLOSE_HUNGUP_FORM_DELAY  value "+ iDelay
                            +" is more than allowed 86400 sec( 24 hrs) ");
                iDelay = 86400;
            }
            else if( iDelay < 0 )
            {
                SysLog.log("PHONESET_CLOSE_HUNGUP_FORM_DELAY  value "+ iDelay
                            +" is less than allowed 0 sec. So set to default of 10 secs. ");
                iDelay = 10;
            }
        }
        return (iDelay*1000);
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Converts an empty String (all white space) into a null.
     */
    protected String convertEmptyStringToNull(String source)
    {
        // Convert empty strings to nulls
        if ((source != null) && (source.trim().length() == 0))
            source = null ;

        return source ;
    }


    /**
     * checks if a string is null or just consists of space
     */
    private boolean isNullOrEmpty(String str)
    {
        boolean bRes = false;

        if(( str == null ) || (str.trim().length() == 0))
            bRes = true;

        return bRes;
    }

    /**
     * gets a value of a property from XPRESSA_VERSION_RESFILE.
     */
    private String getXpressaSettingValue( String strSetting )
    {
        String strRet = null ;

        // Get Build Comment
        try
        {
            PropertyResourceBundle versionBundle = new PropertyResourceBundle
                (ClassLoader.getSystemResourceAsStream(XPRESSA_VERSION_RESFILE));
            strRet = versionBundle.getString(strSetting);
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }
        return strRet;

    }

    /**
     * @deprecated do not expose- this is a testing hook
     */
    public void dump()
    {
        System.out.println("Extension: " + getExtension()) ;
        System.out.println("HTTP Proxy Host: " + getHTTPProxyHost()) ;
        System.out.println("HTTP Proxy Port: " + getHTTPProxyHostPort()) ;
        System.out.println("SIP Proxy Server: " + getSIPProxyServerHost()) ;
        System.out.println("SIP Directory Server: " + getSIPDirectoryServerHost()) ;
        System.out.println("SIP Registry Server: " + getSIPRegistryServerHost()) ;
        System.out.println("SIP TCP Port: " + getSIPTCPPort()) ;
        System.out.println("SIP UDP Port: " + getSIPUDPPort()) ;
        System.out.println("SIP RTP Starting Port: " + getRTPStartingPort()) ;
        System.out.println("HTTP Port: " + getHTTPPort()) ;
        System.out.println("Version: " + getVersion()) ;
        System.out.println("Kernel Version: " + getKernelVersion()) ;
        System.out.println("Product ID: " + getProductID()) ;
    }
}

