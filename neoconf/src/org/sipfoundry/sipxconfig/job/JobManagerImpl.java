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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.collections.buffer.CircularFifoBuffer;

public class JobManagerImpl implements JobManager {
    static final int MAX_JOBS = 50;

    private Collection m_jobs = new CircularFifoBuffer(MAX_JOBS);

    private synchronized Job getJob(Serializable id) {
        if (id instanceof Job) {
            return (Job) id;
        }
        return null;
    }

    private Serializable addNewJob(Job job) {
        job.setUniqueId();
        m_jobs.add(job);
        return job;
    }

    public synchronized Serializable schedule(String name) {
        Job job = new Job(name);
        return addNewJob(job);
    }

    public void start(Serializable jobId) {
        Job job = getJob(jobId);
        if (job != null) {
            job.start();
        }
    }

    public void success(Serializable jobId) {
        Job job = getJob(jobId);
        if (job != null) {
            job.success();
        }
    }

    public void failure(Serializable jobId, String errorMsg, Throwable exception) {
        Job job = getJob(jobId);
        if (job != null) {
            job.failure(errorMsg, exception);
        }
    }

    public synchronized int removeCompleted() {
        int counter = 0;
        for (Iterator i = m_jobs.iterator(); i.hasNext();) {
            Job job = (Job) i.next();
            if (job.getStatus().equals(JobStatus.COMPLETED)) {
                i.remove();
                counter++;
            }
        }
        return counter;
    }

    public synchronized void clear() {
        m_jobs.clear();
    }

    public synchronized List getJobs() {
        return new ArrayList(m_jobs);
    }
}
