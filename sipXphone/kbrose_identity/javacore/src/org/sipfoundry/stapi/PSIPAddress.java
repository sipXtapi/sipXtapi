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

package org.sipfoundry.stapi ;

import java.io.* ;
import java.net.* ;
import java.util.Hashtable ;

import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.util.* ;


/**
 * This class creates a SIP URL, which is used to identify a party in a call.
 * This SIP URL is much like a phone number in that it can be used for dialing
 * as well as to identify a caller.
 * <p>
 * The format of the SIP address is as follows:
 * <PRE>
 * User Label &lt;SIP:user@addr:port ;tranport=protocol&gt; ;tag=nnnn
 * </PRE>
 * Most of the fields in a SIP URL are optional. In its simplest form, you will
 * see only the user and domain. For example:
 * <PRE>
 * sip:rocko@conglomo.com
 * </PRE>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PSIPAddress extends PAddress implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
     transient protected SipParser m_parser ;
     protected boolean m_bUseParser ;



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Protected no-argument constructor provided for serialization.
     */
    protected PSIPAddress()
    {
        m_parser = null ;
        m_bUseParser = true ;
    }


    /**
     * Constructor, takes a SIP URL as a string.
     *
     * @param strSIPURL The SIP URL for the call participant.
     * @exception MalformedURLException Thrown if the SIP URL does not have
     *        the correct syntax.
     */
    public PSIPAddress(String strSIPURL)
        throws MalformedURLException
    {
        super(strSIPURL) ;

        if (!SipParser.isSIPURL(strSIPURL))
            m_bUseParser = false ;

        m_parser = null ;
    }

    /**
     * Constructor, takes individual portions of a SIP URL.
     *
     * @param strUserLabel User Label (also known as Display Name) of the SIP URL.  This data
     *        is optional and generally shows up as caller ID information.
     * @param strUser The username of the call participant.
     * @param strAddr The IP Address or domain name of the participant. For example: 209.251.66.16 or conglomo.com
     *
     */
    public PSIPAddress( String strUserLabel,
                        String strUser,
                        String strAddr )
    {
        super(null) ;

        m_parser = new SipParser() ;

        m_parser.setDisplayName(strUserLabel) ;
        m_parser.setUser(strUser) ;
        m_parser.setHost(strAddr) ;

        setAddress(m_parser.render()) ;
    }


    /**
     * Constructor, takes individual portions of a SIP URL.
     *
     * @param strUserLabel User Label (also known as Display Name) of the SIP URL.  This data
     *        is optional and generally shows up as caller ID information.
     * @param strUser The username of the call participant.
     * @param strAddr The IP Address or domain name of the participant. For example: 209.251.66.16 or conglomo.com
     * @param iPort The TCP/UDP port used by the call participant.
     * @param strTags User specific tags.
     *
     */
    public PSIPAddress( String strUserLabel,
                        String strUser,
                        String strAddr,
                        int    iPort,
                        String strTag)
    {
        super(null) ;

        m_parser = new SipParser() ;

        m_parser.setDisplayName(strUserLabel) ;
        m_parser.setUser(strUser) ;
        m_parser.setHost(strAddr) ;
        m_parser.setPort(iPort) ;
        m_parser.addFieldParameter("tag", strTag) ;

        setAddress(m_parser.render()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Query the user label (display name) of this SIP address.
     *
     * @return The user label for this SIP address.
     */
    public String getUserLabel()
    {
        String strUserLabel = null ;

        if (m_bUseParser)
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getDisplayName() ;
    }


    /**
     * Query the user id of this SIP address.
     *
     * @return The user id of this SIP address.
     */
    public String getUser()
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getUser() ;
    }


    /**
     * Query the address (IP or domain name) of this SIP address.
     *
     * @return The address (IP or domain name) for this SIP address.
     */
    public String getAddr()
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getHost() ;
    }


    /**
     * Query the port number of this SIP address.
     *
     * @return The TCP/UDP port number for this SIP address.
     */
    public int getPort()
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        int iPort = m_parser.getPort() ;

        if (iPort <= 0)
            iPort = 5060 ;

        return iPort ;
    }


    /**
     * Query the tag of this SIP address.
     *
     * @return The tag for this SIP address.
     */
    public String getTag()
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getFieldParameterValue("tag") ;
    }


    /**
     * Query the tag of this SIP address.
     *
     * @return The tag for this SIP address.
     *
     * @deprecated this has been deprecated.  Use getTag()
     */
    public String getTags()
    {
        return getTag() ;
    }


    /**
     * Get a named header parameter for the SIP address.
     *
     * @return The requested parameter or null if not found.
     */
    public String getHeaderParameter(String strName)
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getHeaderParameterValue(strName) ;
    }


    /**
     * Get all of the header parameters defined for this address.
     *
     * @return Hashtable of name/value pairs.
     */
    public Hashtable getHeaderParameters()
    {
        Hashtable htRC = new Hashtable() ;

        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        String params[] = m_parser.getHeaderParameters() ;
        if (params != null)
        {
            for (int i=0; i<params.length; i++)
            {
                if (!htRC.contains(params[i]))
                {
                    String strValue = getHeaderParameter(params[i]) ;
                    if (strValue == null)
                        strValue = new String() ;
                    htRC.put(params[i], strValue) ;
                }
            }
        }

        return htRC ;
    }


    /**
     * Get a named URL parameter for the SIP address.
     *
     * @return The requested parameter or null if not found.
     */
    public String getURLParameter(String strName)
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getURLParameterValue(strName) ;
    }


    /**
     * Get all of the URL parameters defined for this address.
     *
     * @return Hashtable of name/value pairs.
     */
    public Hashtable getURLParameters()
    {
        Hashtable htRC = new Hashtable() ;

        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        String params[] = m_parser.getURLParameters() ;
        if (params != null)
        {
            for (int i=0; i<params.length; i++)
            {
                if (!htRC.contains(params[i]))
                {
                    String strValue = getURLParameter(params[i]) ;
                    if (strValue == null)
                        strValue = new String() ;
                    htRC.put(params[i], strValue) ;
                }
            }
        }

        return htRC ;
    }


    /**
     * Get a named Field parameter for the SIP address.
     *
     * @return The requested parameter or null if not found.
     */
    public String getFieldParameter(String strName)
    {
        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        return m_parser.getFieldParameterValue(strName) ;

    }


    /**
     * Get all of the Field parameters defined for this address.
     *
     * @returns hash table of field parameters
     */
    public Hashtable getFieldParameters()
    {
        Hashtable htRC = new Hashtable() ;

        if (m_parser == null)
            m_parser = new SipParser(getAddress()) ;

        String params[] = m_parser.getFieldParameters() ;
        if (params != null)
        {
            for (int i=0; i<params.length; i++)
            {
                if (!htRC.contains(params[i]))
                {
                    String strValue = getFieldParameter(params[i]) ;
                    if (strValue == null)
                        strValue = new String() ;
                    htRC.put(params[i], strValue) ;
                }
            }
        }

        return htRC ;
    }



    /**
     * Determine if one address is equal to another, ignoring parameters
     * (except tag) and display name
     *
     * @deprecated do not expose
     */
    public boolean equals(Object obj)
    {
        boolean bRC = false ;

        if ((obj != null) && (obj instanceof PSIPAddress))
        {
            PSIPAddress address = (PSIPAddress) obj ;
            bRC = ( safeStringCompare(getUser(), address.getUser()) &&
                safeStringCompare(getAddr(), address.getAddr()) &&
                compareTags(getTag(), address.getTag()) &&
                getPort() == address.getPort() ) ;
        }
        else if (obj != null)
            bRC = super.equals(obj) ;

        return bRC ;
    }


    /**
     * Determine the hash code of this object.
     *
     * @deprecated do not expose
     */
    public int hashCode()
    {
        if ((getUser() == null) && (getAddr() == null) && (getTag() == null) && (getPort() == 5060))
            return super.hashCode() ;
        else {
            int hash = 0 ;

            if (getUser() != null)
                hash ^= getUser().hashCode() ;
            if (getAddr() != null)
                hash ^= getAddr().hashCode() ;
            if (getTag() != null)
                hash ^= getTag().hashCode() ;

            hash += getPort() ;

            return hash ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Safely determine if the two specified strings are identical.
     */
    private boolean safeStringCompare(String strSource, String strTarget)
    {
        if ((strSource == null) && (strTarget != null))
            return false ;

        if ((strSource != null) && (strTarget == null))
            return false ;

        if ((strSource == null) && (strTarget == null))
            return true ;
        else
            return strSource.equalsIgnoreCase(strTarget) ;
    }



    /**
     * Determine if two tags match.  If one of the tags is null, then the tags
     * will match, otherwise, the tags will match if the tags match
     * identically.
     */
    private boolean compareTags(String strSourceTag, String strTargetTag)
    {
        if ((strSourceTag == null) && (strTargetTag != null))
            return true ;

        if ((strSourceTag != null) && (strTargetTag == null))
            return true ;

        if ((strSourceTag == null) && (strTargetTag == null))
            return true ;
        else
            return strSourceTag.equalsIgnoreCase(strTargetTag) ;
    }
}
