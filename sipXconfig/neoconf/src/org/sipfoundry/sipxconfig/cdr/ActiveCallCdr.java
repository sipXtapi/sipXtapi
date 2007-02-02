/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

/**
 * Special type of CDR to present active calls. Duration is set explicitly (from call resolver),
 * termination code is always in progress.
 */
public class ActiveCallCdr extends Cdr {
    private long m_duration;

    public void setDuration(long duration) {
        m_duration = duration;
    }

    public long getDuration() {
        return m_duration;
    }

    public Termination getTermination() {
        return Cdr.Termination.IN_PROGRESS;
    }
}
