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

import java.util.Date;

/**
 * Database object
 */
public class Job {

    private int m_id;

    private int m_type;

    private String m_status;

    private Date m_startTime;

    private String m_details;

    private String m_progress;

    private String m_exceptionMessage;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        this.m_id = id;
    }

    public int getType() {
        return m_type;
    }

    public void setType(int type) {
        this.m_type = type;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        this.m_status = status;
    }

    public Date getStartTime() {
        return m_startTime;
    }

    public void setStartTime(Date startTime) {
        this.m_startTime = startTime;
    }

    public String getDetails() {
        return m_details;
    }

    public void setDetails(String details) {
        this.m_details = details;
    }

    public String getProgress() {
        return m_progress;
    }

    public void setProgress(String progress) {
        this.m_progress = progress;
    }

    public String getExceptionMessage() {
        return m_exceptionMessage;
    }

    public void setExceptionMessage(String exceptionMessage) {
        this.m_exceptionMessage = exceptionMessage;
    }

}
