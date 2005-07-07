/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipRequest.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.sip ;

import java.util.Vector;
import java.util.StringTokenizer;
import java.io.InputStream;
import java.io.IOException;
import java.io.Serializable;


/**
 * This class extends a generic SipMessage class to provide accessor and
 * mutator methods that are specific to sip requests.
 *
 * @author Daniel Petrie
 */
public class SipRequest extends SipMessage implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default public no argument constructor required for serialization
     */
    public SipRequest()
    {
        super() ;
    }


    /**
     * Constructor accepting a InputStream as a source of the Sip Message.
     * This object will populated from the text data read from the designated
     * stream.
     *
     * @param inputstream InputStream containing a text of a Sip Message.
     */
    public SipRequest(InputStream inputstream)
        throws IOException
    {
        super(inputstream) ;
    }


    /**
     * Constructor accepting a string as the source of the Sip Message.
     *
     * @param strMessage Text of a Sip Message
     */
    public SipRequest(String strMessage)
    {
        super(strMessage) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Gets the request method
     */
    public String getMethod()
    {
        String strMethod = (new StringTokenizer(header)).nextToken() ;

        return strMethod ;
    }


    /**
     * Gets the request URI
     */
    public String getURI()
    {
        StringTokenizer tokenizer = new StringTokenizer(header) ;
        tokenizer.nextToken() ;

        String strURI = tokenizer.nextToken() ;
        return strURI ;
    }


    /**
     * Sets the first line of the request header
     *
     * @param strMethod the request method name
     * @param strURI the request URI
     */
    public void setRequestHeader(String strMethod, String strURI)
    {
        setHeader(strMethod + " " + strURI + " " + SIP_PROTOCOL);
    }


    /**
     * Build a response message to this request.
     *
     * @param iResponseCode The desired response code
     * @param strResponseText The desired response text
     */
    public SipResponse buildResponse(int iResponseCode, String strResponseText)
    {
        return JNI_buildResponse(toString(), iResponseCode, strResponseText) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////


//////////////////////////////////////////////////////////////////////////////
// Native methods
////
    private static native SipResponse JNI_buildResponse(String strRequestText,
            int iResponseCode, String strResponseText) ;
}
