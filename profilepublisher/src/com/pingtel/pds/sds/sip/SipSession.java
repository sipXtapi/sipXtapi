/*
 * 
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

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class SipSession extends Object {
    private String m_toAddress;
    private String m_fromAddress;
    private String m_callId;
    private String m_event;

    /**
     * Override the default implementation to use the SipSession
     * as a unique key to a hashtable
     */
    public int hashCode() {
        return (m_fromAddress + m_toAddress + m_callId + m_event).hashCode();
    }

    /**
     * Override the default implementation to use the SipSession
     * as a unique key to a hashtable
     */
    public boolean equals( Object arg ) {
        if (arg instanceof SipSession)
            return this.hashCode() == ((SipSession)arg).hashCode();
        else
            return false;
    }

    /**
     *
     * @param toAddress
     * @param fromAddress
     * @param callId
     */
    public SipSession( String fromAddress,
                       String toAddress,
                       String callId,
                       String event) {
        m_fromAddress = fromAddress;
        m_toAddress = toAddress;
        m_callId = callId;
        m_event = event;
    }

    /**
     *
     * @return
     */
    public String getFromAddress() {
        return m_fromAddress;
    }

    /**
     *
     * @return
     */
    public String getToAddress() {
        return m_toAddress;
    }

    /**
     *
     * @return
     */
    public String getCallId() {
        return m_callId;
    }

    /**
     *
     * @return
     */
    public String getEvent() {
        return m_event;
    }
}