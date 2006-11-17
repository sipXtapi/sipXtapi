/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.util.Date;

public class Cdr {
    enum Termination {
        UNKNOWN, REQUESTED, IN_PROGRESS, COMPLETED, FAILED;

        public static Termination fromString(String t) {
            switch (t.charAt(0)) {
            case 'R':
                return REQUESTED;
            case 'C':
                return COMPLETED;
            case 'F':
                return FAILED;
            case 'I':
                return IN_PROGRESS;
            default:
                return UNKNOWN;
            }
        }
    }

    private String m_callerAor;
    private String m_calleeAor;

    private Date m_startTime;
    private Date m_connectTime;
    private Date m_endTime;

    private Termination m_termination;
    private int m_failureStatus;

    public String getCalleeAor() {
        return m_calleeAor;
    }

    public void setCalleeAor(String calleeAor) {
        m_calleeAor = calleeAor;
    }

    public String getCallerAor() {
        return m_callerAor;
    }

    public void setCallerAor(String callerAor) {
        m_callerAor = callerAor;
    }

    public Date getConnectTime() {
        return m_connectTime;
    }

    public void setConnectTime(Date connectTime) {
        m_connectTime = connectTime;
    }

    public Date getEndTime() {
        return m_endTime;
    }

    public void setEndTime(Date endTime) {
        m_endTime = endTime;
    }

    public int getFailureStatus() {
        return m_failureStatus;
    }

    public void setFailureStatus(int failureStatus) {
        m_failureStatus = failureStatus;
    }

    public Date getStartTime() {
        return m_startTime;
    }

    public void setStartTime(Date startTime) {
        m_startTime = startTime;
    }

    public Termination getTermination() {
        return m_termination;
    }

    public void setTermination(Termination termination) {
        m_termination = termination;
    }

    public long getDuration() {
        if (m_endTime == null || m_connectTime == null) {
            return 0;
        }
        return m_endTime.getTime() - m_connectTime.getTime();
    }
}
