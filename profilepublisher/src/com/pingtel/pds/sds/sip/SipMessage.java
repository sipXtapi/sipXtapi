/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/sip/SipMessage.java#6 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.sip;

import java.io.* ;
import java.util.* ;
import org.apache.log4j.Category;

/**
 * A SipMessage add some convenience accessor methods to a HttpMessage
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipMessage extends HttpMessage implements Serializable
{
    /** define the Xpressa notify specific headers  */
    public static final String SIP_NOTIFY_DEVICE = "x-xpressa-device";
    public static final String SIP_NOTIFY_USER = "x-xpressa-user";
    public static final String SIP_NOTIFY_APPS = "x-xpressa-apps";
    public static final String SIP_NOTIFY_INSTALL = "x-xpressa-install";
    public static final String SIP_LEGACY_SUBSCRIBE_EVENT = "sip-config";

    /** define the subscribe specific headers  */
    public static final String SIP_SUBSCRIBE_MACADDRESS = "Mac";
    public static final String SIP_SUBSCRIBE_VENDOR = "Vendor";
    public static final String SIP_SUBSCRIBE_MODEL = "Model";
    public static final String SIP_SUBSCRIBE_SERIALNUMBER = "Serial";
    public static final String SIP_SUBSCRIBE_VERSION = "Version";
    public static final String SIP_SUBSCRIBE_EXPIRATION_SECS = "Expires";
    public static final String SIP_SUBSCRIBE_ALLOWED_PROTOCOLS  = "Config_Allow";
    public static final String SIP_SUBSCRIBE_ALLOWED_PROTOCOLS1  = "Config_Allow";
    public static final String SIP_SUBSCRIBE_REQUIRED_PROFILES  = "Config_Require";
    public static final String SIP_SUBSCRIBE_REQUIRED_PROFILES1  = "Config-Require";

    /** define headers used for both subscribe and notify */
    public static final String SIP_TO_ADDRESS = "To";
    public static final String SIP_FROM_ADDRESS = "From";
    public static final String SIP_CALL_ID = "Call-Id";
    public static final String SIP_CSEQ = "CSeq";
    public static final String SIP_EVENT = "Event";
    public static final String SIP_CONTACT = "Contact";

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor requires for serialization
     */
    public SipMessage() {
        super() ;
    }


    /**
     * Constructor accepting an input stream as the source of the sip message
     */
    public SipMessage(InputStream iStream) throws IOException  {
        super(iStream) ;
    }


    /**
     * Constructor accepting a string as the source  of the sip message
     */
    public SipMessage(String strMessage) {
        super(strMessage) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the sequence number for this request field
     *
     * @return the CSeq number or -1 on error
     */
    public int getCSequenceNumber()
    {
        int iCSeq = -1 ;

        String strValue = getHeaderFieldValue(SIP_CSEQ) ;
        if (strValue != null) {
            StringTokenizer toker = new StringTokenizer(strValue) ;
            if (toker.hasMoreElements()) {
                String strSequenceNumber = toker.nextToken() ;
                if (strSequenceNumber != null) {
                    try {
                        iCSeq = Integer.parseInt(strSequenceNumber) ;
                    } catch (NumberFormatException nfe) {
                        m_logger.error( "Invalid CSeq Field: ", nfe );
                    }
                }
            }
        }
        return iCSeq ;
    }


    /**
     * Get the method for this messages's CSeq field
     */
    public String getCSequenceMethod() {
        String strMethod = null ;

        String strValue = getHeaderFieldValue(SIP_CSEQ);
        if (strValue != null) {
            StringTokenizer toker = new StringTokenizer(strValue) ;
            if (toker.hasMoreElements()) {
                toker.nextToken() ;
                if (toker.hasMoreElements()) {
                    strMethod = toker.nextToken("\n") ;
                }
            }
        }
        return strMethod ;
    }
}
