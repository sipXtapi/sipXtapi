/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.job;

import java.io.Serializable;
import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Information about the job
 */
public class Job extends BeanWithId implements Serializable {
    private Date m_start;

    private Date m_stop;

    private JobStatus m_status = JobStatus.SCHEDULED;

    private String m_name;

    private String m_errorMsg = StringUtils.EMPTY;

    private Throwable m_exception;

    public Job(String name) {
        m_name = name;
    }

    Job() {
        this(StringUtils.EMPTY);
    }

    public void start() {
        if (!m_status.equals(JobStatus.SCHEDULED)) {
            throw new IllegalStateException();
        }
        m_start = new Date();
        m_status = JobStatus.IN_PROGRESS;
    }

    public void success() {
        stop(JobStatus.COMPLETED);
    }

    public void failure(String errorMsg, Throwable e) {
        stop(JobStatus.FAILED);
        m_errorMsg = errorMsg;
        m_exception = e;
    }

    // should be private - checkstyle complains
    protected void stop(JobStatus status) {
        if (!m_status.equals(JobStatus.IN_PROGRESS)) {
            throw new IllegalStateException();
        }
        m_stop = new Date();
        m_status = status;
    }

    public String getName() {
        return m_name;
    }

    public Date getStart() {
        return m_start;
    }

    public Date getStop() {
        return m_stop;
    }

    public JobStatus getStatus() {
        return m_status;
    }

    public String getErrorMsg() {
        if (!m_status.equals(JobStatus.FAILED)) {
            return org.apache.commons.lang.StringUtils.EMPTY;
        }
        StringBuffer error = new StringBuffer();
        if (m_errorMsg != null) {
            error.append(m_errorMsg);
        }
        if (m_exception != null) {
            error.append('\n');
            error.append(m_exception.getLocalizedMessage());
        }
        return error.toString();
    }
}
