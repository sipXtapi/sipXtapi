/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

/**
 * Database object
 */
public class ProfileData {

    private int m_logPnId;

    private int m_type;

    private int m_seqNumber;

    public int getLogPnId() {
        return m_logPnId;
    }

    public void setLogPnId(int logPnId) {
        m_logPnId = logPnId;
    }

    public int getType() {
        return m_type;
    }

    public void setType(int type) {
        m_type = type;
    }

    public int getSeqNumber() {
        return m_seqNumber;
    }

    public void setSeqNumber(int seqNumber) {
        m_seqNumber = seqNumber;
    }

}
