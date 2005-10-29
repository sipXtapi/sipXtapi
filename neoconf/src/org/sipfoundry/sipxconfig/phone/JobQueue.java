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
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BackgroundTaskQueue;
import org.sipfoundry.sipxconfig.job.JobManager;

public class JobQueue extends BackgroundTaskQueue {
    private static final Log LOG = LogFactory.getLog(JobQueue.class);

    private JobManager m_jobContext;

    public void addJob(JobRecord job) {
        Phone[] phones = job.getPhones();
        for (int i = 0; i < phones.length; i++) {
            Job phoneJob = new Job(phones[i], job.getType(), m_jobContext);
            addTask(phoneJob);
        }
    }

    static class Job implements Runnable {
        private Phone m_phone;
        private int m_type;
        private JobManager m_jobContext;

        public Job(Phone phone, int type, JobManager context) {
            m_phone = phone;
            m_type = type;
            m_jobContext = context;
        }

        public void run() {
            Serializable jobId = m_jobContext.schedule("Projection for phone "
                    + m_phone.getSerialNumber());
            try {
                m_jobContext.start(jobId);
                if (m_type == JobRecord.TYPE_PROJECTION) {
                    m_phone.generateProfiles();
                }
                m_phone.restart();
                m_jobContext.success(jobId);
            } catch (RestartException re) {
                m_jobContext.failure(jobId, null, re);
            } catch (RuntimeException e) {
                m_jobContext.failure(jobId, null, e);
                // do not throw error, job queue will stop running.
                // error gets logged to job error table and sipxconfig.log
                LOG.error(e);
            }
        }
    }

    public void setJobContext(JobManager jobContext) {
        m_jobContext = jobContext;
    }
}
