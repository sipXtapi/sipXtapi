/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipSession.java#2 $
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

import java.io.PrintStream ;


/**
 * The SipSession object defines the three pieces of data needed to indentify
 * a SIP session: CallID, Remote URI, and Local URI.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipSession extends org.sipfoundry.stapi.PSession
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String m_strCallID ;    // Call ID for the Session
    private String m_strRemoteURL ;     // remote URL for the Session
    private String m_strLocalURL ;   // local URL for the Session
    private String m_strLocalContact;   // Local Contact Address


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor requiring the Call ID, Remote URL, and Local URL.
     *
     * @param strCallID  Call ID for the session
     * @param strRemoteURL   Remote URL for the session
     * @param strLocalURL Local URL for the session
     */
    public SipSession(String strCallID,
                      String strRemoteURL,
                      String strLocalURL)
    {
        this(strCallID, strRemoteURL, strLocalURL, null) ;
    }


    /**
     * Constructor requiring the Call ID, Remote URL, and Local URL, and
     * local contact URL
     *
     * @param strCallID  Call ID for the session
     * @param strRemoteURL   Remote URL for the session
     * @param strLocalURL Local URL for the session
     * @param strLocalContact Local Contact for the session
     */
    public SipSession(String strCallID,
                      String strRemoteURL,
                      String strLocalURL,
                      String strLocalContact)
    {
        m_strCallID = strCallID ;
        m_strRemoteURL = strRemoteURL ;
        m_strLocalURL = strLocalURL ;
        m_strLocalContact = strLocalContact ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the Call for the session.
     *
     * @return Call ID for the session.
     */
    public String getCallID()
    {
        return m_strCallID ;
    }


    /**
     * Get the Remote URL for the session.
     *
     * @return Remote URL for the session.
     */
    public String getRemoteURL()
    {
        return m_strRemoteURL ;
    }


    /**
     * Get the Local URL for the session.
     *
     * @return Local URL for the session.
     */
    public String getLocalURL()
    {
        return m_strLocalURL ;
    }

    /**
     * Get the Remote URL for the session.
     *
     * @return Remote URL for the session.
     * @deprecated use getRemoteURL() instead. Placed here
     * for backward compatibility.
     */
    public String getToURL()
    {
        return m_strRemoteURL ;
    }


    /**
     * Get the Local URL for the session.
     *
     *
     * @return Local URL for the session.
     * @deprecated use getLocalURL() instead.Placed here
     * for backward compatibility.
     */
    public String getFromURL()
    {
        return m_strLocalURL ;
    }


    /**
     * Get the Local Contact URL for the session
     *
     * @return Local contact URL for the session
     */
    public String getLocalContact()
    {
        return m_strLocalContact ;
    }


    /**
     * Get the next cseq number for this session.
     *
     * @return next CSeq number for this session
     * @deprecated
     */
    public int getNextCSeqNumber()
    {
        return JNI_getNextCSeqNumber(m_strCallID, m_strRemoteURL) ;
    }

    /**
     * Get the next cseq number.
     *
     * @return next CSeq number
     * @deprecated
     */
    public static int getNextCSeqNumber(String strCallID, String strRemoteURL)
    {
        return JNI_getNextCSeqNumber(strCallID, strRemoteURL) ;
    }


    /**
     * Display contents of session to the specified print stream
     *
     * @deprecate Do not expose
     */
    public void print(PrintStream out)
    {
        out.println("--Sip Session--") ;
        out.println("  CallID: " + m_strCallID) ;
        out.println("  Remote URL: " + m_strRemoteURL)  ;
        out.println("  Local URL: " + m_strLocalURL) ;
        out.println("  Local Contact: " + m_strLocalContact) ;
        out.println("") ;
    }


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
     /**
      * jni method to get the nect CSeq number.
      */
     private static native int JNI_getNextCSeqNumber
            (String strCallID, String strRemoteAddress) ;

}
