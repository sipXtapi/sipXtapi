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
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class JobContextImpl implements JobContext {
    private List m_jobs = new ArrayList();

    private synchronized Job getJob(Serializable id) {
        int i = m_jobs.indexOf(new BeanWithId((Integer) id));
        return (Job) m_jobs.get(i);
    }

    public synchronized Serializable schedule(String name) {
        Job job = new Job(name);
        // temporary - until we implement database storage
        job.setUniqueId();
        m_jobs.add(job);
        return job.getId();
    }

    public void start(Serializable jobId) {
        getJob(jobId).start();
    }

    public void success(Serializable jobId) {
        getJob(jobId).success();
    }

    public void failure(Serializable jobId, String errorMsg, Throwable exception) {
        getJob(jobId).failure(errorMsg, exception);
    }

    public synchronized void removeCompleted() {
        for (Iterator i = m_jobs.iterator(); i.hasNext();) {
            Job job = (Job) i.next();
            if (job.getStatus().equals(JobStatus.COMPLETED)) {
                i.remove();
            }
        }
    }

    public synchronized void clear() {
        m_jobs.clear();
    }

    public synchronized List getJobs() {
        return new ArrayList(m_jobs);
    }
}
