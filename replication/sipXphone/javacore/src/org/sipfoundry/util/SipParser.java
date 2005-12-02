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

package org.sipfoundry.util ;

import java.util.Hashtable ;
import org.sipfoundry.stapi.PAddress ;

/**
 * This class is used to parse SIP addresses.
 * To use this class, you call the contructor that takes a string as
 * an argument. This string is the SIP address that you want to parse.
 *
 * Once the object is constructed, you can call the get methods to get each field
 * of the string.
 *
 * The format of the SIP address is as follows:
 * User Label <SIP:user@address:port ;tranport=protocol> ;tag=nnnn
 *
 * The more complete form, that we don't support, is this:
 * [[["]UserName["]] <] sip: [UserId@] IpAddressOrDns [:Port] [;UrlToken = ["]value["]] [? [HeaderField = ["]value["]]] [>] [; FieldToken = ["]value["] [;]]
 *
 * UserName can have spaces regardless of whether it is quoted
 * If UserName is specified > must be matched
 * There may be 0 or more UrlToken and HeaderField pairs.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipParser
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private long m_lHandle ;    // Handle to underlying C++ parser


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking existing sip url
     */
    public SipParser(String strURL) throws IllegalArgumentException
    {
        this() ;

        reset(strURL) ;
    }


    /**
     * default no argument constructor
     */
    public SipParser()
    {
        m_lHandle = JNI_createParser() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////
    /**
     * what is the address of our SIP URL?
     */
    public String getHost()
    {
        return JNI_getHost(m_lHandle, 0) ;
    }

     /**
     * returns the identity of this SIP URL. An identity is username@host:port .
     */
    public String getIdentity()
    {
        return JNI_getIdentity(m_lHandle, 0) ;
    }

    /**
     * @deprecated - do not expose
     */
    public String getAddress()
    {
        return getHost() ;
    }


    /**
     * set an address for this SIP URL.
     */
    public void setHost(String strAddress)
    {
        JNI_setHost(m_lHandle, strAddress) ;
    }


    /**
     * what is the port of our SIP URL?
     */
    public int getPort()
    {
        return JNI_getPort(m_lHandle, 0) ;
    }


    /**
     * set a port for this SIP URL.
     */
    public void setPort(int iPort)
    {
        JNI_setPort(m_lHandle, iPort) ;
    }


    /**
     * what is the protocol of our URL?
     */
    public String getScheme()
    {
        return JNI_getScheme(m_lHandle, 0) ;
    }


    /**
     * set a protocol for this SIP URL.
     */
    public void setScheme(String strScheme)
    {
        JNI_setScheme(m_lHandle, strScheme) ;
    }


    /**
     * what is the username of our URL
     */
    public String getUser()
    {
        return JNI_getUser(m_lHandle, 0) ;
    }


    /**
     * set a user id for this SIP URL.
     */
    public void setUser(String strUser)
    {
        JNI_setUser(m_lHandle, strUser) ;
    }


    /**
     * what is the user label of our URL
     */
    public String getDisplayName()
    {
        return JNI_getDisplayName(m_lHandle, 0) ;
    }


    /**
     * set a user label for this SIP URL.
     */
    public void setDisplayName(String strDisplayName)
    {
        JNI_setDisplayName(m_lHandle, strDisplayName) ;
    }

    //
    // Field Parameters
    //

    public String[] getFieldParameters()
    {
        return JNI_getFieldParameters(m_lHandle, 0) ;
    }


    public String[] getFieldParameterValues(String strName)
    {
        return JNI_getFieldParameterValues(m_lHandle, strName) ;
    }


    public String getFieldParameterValue(String strName)
    {
        String strValue = null ;

        String strValues[] = getFieldParameterValues(strName) ;
        if ((strValues != null) && (strValues.length > 0))
            strValue = strValues[0] ;

        return strValue ;
    }


    public void addFieldParameter(String strName, String strValue)
    {
        JNI_addFieldParameter(m_lHandle, strName, strValue) ;
    }


    public void removeFieldParameters(String strName)
    {
        JNI_removeFieldParameters(m_lHandle, strName) ;
    }


    public void removeAllFieldParameters()
    {
        JNI_removeAllFieldParameters(m_lHandle, 0) ;
    }


    //
    // URL Parameters
    //


    public String[] getURLParameters()
    {
        return JNI_getURLParameters(m_lHandle, 0) ;
    }


    public String[] getURLParameterValues(String strName)
    {
        return JNI_getURLParameterValues(m_lHandle, strName) ;
    }


    public String getURLParameterValue(String strName)
    {
        String strValue = null ;

        String strValues[] = getURLParameterValues(strName) ;
        if ((strValues != null) && (strValues.length > 0))
            strValue = strValues[0] ;

        return strValue ;
    }


    public void addURLParameter(String strName, String strValue)
    {
        JNI_addURLParameter(m_lHandle, strName, strValue) ;
    }


    public void removeURLParameters(String strName)
    {
        JNI_removeURLParameters(m_lHandle, strName) ;
    }


    public void removeAllURLParameters()
    {
        JNI_removeAllURLParameters(m_lHandle, 0) ;
    }

    //
    // Header Parameters
    //


    public String[] getHeaderParameters()
    {
        return JNI_getHeaderParameters(m_lHandle, 0) ;
    }


    public String[] getHeaderParameterValues(String strName)
    {
        return JNI_getHeaderParameterValues(m_lHandle, strName) ;
    }


    public String getHeaderParameterValue(String strName)
    {
        String strValue = null ;

        String strValues[] = getHeaderParameterValues(strName) ;
        if ((strValues != null) && (strValues.length > 0))
            strValue = strValues[0] ;

        return strValue ;
    }


    public void addHeaderParameter(String strName, String strValue)
    {
        JNI_addHeaderParameter(m_lHandle, strName, strValue) ;
    }


    public void removeHeaderParameters(String strName)
    {
        JNI_removeHeaderParameters(m_lHandle, strName) ;
    }

    public void removeAllHeaderParameters()
    {
        JNI_removeAllHeaderParameters(m_lHandle, 0) ;
    }

    /**
     * Are angle brackets included when rendered?
     */
    public boolean isIncludeAngleBracketsSet()
    {
        return JNI_isIncludeAngleBracketsSet(m_lHandle, 0) ;
    }


    /**
     * Should angle brackets be included when rendering.
     */
    public void setIncludeAngleBrackets(boolean bInclude)
    {
        JNI_setIncludeAngleBrackets(m_lHandle, bInclude) ;
    }


    //
    // Render / reset
    //

    public String render()
    {
        return JNI_render(m_lHandle, 0) ;
    }


    public void reset(String strURL)
    {
        JNI_resetContent(m_lHandle, strURL) ;
    }


/////////////////////////////////////////////////////////////////////////////
// Static Methods
////
    public static boolean isSIPURL(String strSIPURL)
    {
        boolean bIsSipURL = false ;

        if (strSIPURL != null)
        {
            String strCompareTo = strSIPURL.toUpperCase() ;
            if ((strCompareTo.indexOf("SIP:") >= 0) || (strCompareTo.indexOf("TEL:") >= 0))
            {
                bIsSipURL = true ;
            }
        }

        return bIsSipURL ;
    }

    public static String stripParameters(String strSIPURL)
    {
        String strAddress = strSIPURL ;

        if ((strSIPURL != null) && isSIPURL(strSIPURL))
        {
            SipParser parser = new SipParser(strSIPURL) ;
            parser.removeAllFieldParameters() ;
            parser.removeAllURLParameters() ;
            parser.removeAllHeaderParameters() ;

            parser.setIncludeAngleBrackets(true) ;
            strAddress = parser.render() ;
        }

        return strAddress ;
    }


    public static String stripFieldParameters(String strSIPURL)
    {
        String strAddress = strSIPURL ;

        if ((strSIPURL != null) && isSIPURL(strSIPURL))
        {
            SipParser parser = new SipParser(strSIPURL) ;
            parser.removeAllFieldParameters() ;

            parser.setIncludeAngleBrackets(true) ;
            strAddress = parser.render() ;
        }

        return strAddress ;
    }

    public static String stripParameters(org.sipfoundry.stapi.PAddress address)
    {
        String strAddress = null ;

        if (address != null)
        {
            if (isSIPURL(address.getAddress()))
            {
                SipParser parser = new SipParser(address.getAddress()) ;
                parser.removeAllFieldParameters() ;
                parser.removeAllURLParameters() ;
                parser.removeAllHeaderParameters() ;

                parser.setIncludeAngleBrackets(true) ;
                strAddress = parser.render() ;
            }
            else
            {
                strAddress = address.getAddress() ;
            }
        }

        return strAddress ;
    }


    public static String stripParameters(org.sipfoundry.telephony.PtAddress address)
    {
        String strAddress = null ;

        if (address != null)
        {
            if (isSIPURL(address.getName()))
            {
                SipParser parser = new SipParser(address.getName()) ;
                parser.removeAllFieldParameters() ;
                parser.removeAllURLParameters() ;
                parser.removeAllHeaderParameters() ;

                parser.setIncludeAngleBrackets(true) ;
                strAddress = parser.render() ;
            }
            else
            {
                strAddress = address.getName() ;
            }
        }

        return strAddress ;
    }


    /**
     * Cleans an address and returns it as a string
     */
    public static String cleanAddress(org.sipfoundry.stapi.PAddress address, boolean bKeepTag)
    {
        String strAddress = address.toString() ;

        // Add the state
        if (SipParser.isSIPURL(strAddress))
        {
            try
            {
                SipParser parser = new SipParser(strAddress) ;

                parser.removeAllHeaderParameters() ;
                parser.removeAllURLParameters() ;
                parser.setDisplayName("") ;

                if (!bKeepTag)
                    parser.removeAllFieldParameters() ;

                parser.setIncludeAngleBrackets(true) ;
                strAddress = parser.render() ;
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }

        return strAddress ;
    }


    /**
     * Determine if the address are identical
     */
    public static boolean areAddressesIdentical(PAddress src,
                                                PAddress trg)
    {
        String strSrc = SipParser.cleanAddress(src, true) ;
        String strTrg = SipParser.cleanAddress(trg, true) ;

        return strSrc.equalsIgnoreCase(strTrg) ;
    }


    public static boolean areAddressesEquivalent(PAddress src,
                                                 PAddress trg)
    {
        boolean bContainsTag = false ;
        String strSrc = src.toString() ;
        String strTrg ;

        // Add the state
        if (SipParser.isSIPURL(strSrc))
        {
            try
            {
                SipParser parser = new SipParser(strSrc) ;

                parser.removeAllHeaderParameters() ;
                parser.removeAllURLParameters() ;
                parser.setDisplayName("") ;

                if (parser.getFieldParameterValue("tag") != null)
                    bContainsTag = true ;

                strSrc = parser.render() ;
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }

        strTrg = SipParser.cleanAddress(trg, bContainsTag) ;

        return strSrc.equalsIgnoreCase(strTrg) ;
    }




/////////////////////////////////////////////////////////////////////////////
// Implementation methods
////
    protected void finalize()
        throws Throwable
    {
        if (m_lHandle != 0)
        {
            JNI_destroyParser(m_lHandle, 0) ;
            m_lHandle = 0 ;
        }
    }



/////////////////////////////////////////////////////////////////////////////
// Native Methods
////s
    protected static native long   JNI_createParser() ;
    protected static native void   JNI_destroyParser(long lHandle, int unused) ;

    protected static native void   JNI_resetContent(long loHandle, String strURL) ;
    protected static native String JNI_render(long lHandle, int unused) ;

    protected static native boolean JNI_isIncludeAngleBracketsSet(long lHandle, int unused) ;
    protected static native void    JNI_setIncludeAngleBrackets(long lHandle, boolean bInclude) ;

    protected static native String JNI_getScheme(long lHandle, int unused) ;
    protected static native void   JNI_setScheme(long lHandle, String strScheme) ;

    protected static native String JNI_getIdentity(long lHandle, int unused) ;

    protected static native String JNI_getHost(long lHandle, int unused) ;
    protected static native void   JNI_setHost(long lHandle, String strHost) ;

    protected static native int    JNI_getPort(long lHandle, int unused) ;
    protected static native void   JNI_setPort(long lHandle, int iPort) ;

    protected static native String JNI_getUser(long lHandle, int unused) ;
    protected static native void   JNI_setUser(long lHandle, String strUser) ;

    protected static native String JNI_getDisplayName(long lHandle, int unused) ;
    protected static native void   JNI_setDisplayName(long lHandle, String strDisplayName) ;

    protected static native String[] JNI_getFieldParameters(long lHandle, int unused) ;
    protected static native String[] JNI_getURLParameters(long lHandle, int unused) ;
    protected static native String[] JNI_getHeaderParameters(long lHandle, int unused) ;

    protected static native String[] JNI_getFieldParameterValues(long lHandle, String strName) ;
    protected static native String[] JNI_getURLParameterValues(long lHandle,  String strName) ;
    protected static native String[] JNI_getHeaderParameterValues(long lHandle, String strName) ;

    protected static native void JNI_addFieldParameter(long lHandle, String strName, String strValue) ;
    protected static native void JNI_addURLParameter(long lHandle, String strName, String strValue) ;
    protected static native void JNI_addHeaderParameter(long lHandle, String strName, String strValue) ;

    protected static native void JNI_removeFieldParameters(long lHandle, String strName) ;
    protected static native void JNI_removeURLParameters(long lHandle, String strName) ;
    protected static native void JNI_removeHeaderParameters(long lHandle, String strName) ;

    protected static native void JNI_removeAllFieldParameters(long lHandle, int unused) ;
    protected static native void JNI_removeAllURLParameters(long lHandle, int unused) ;
    protected static native void JNI_removeAllHeaderParameters(long lHandle, int unused) ;
}




