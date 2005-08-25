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
import org.sipfoundry.sipxconfig.job.JobContext;

/**
 * Run an ansynchorous operation on a list of jobs.
 */
public class JobManagerImpl extends Thread implements JobManager {

    private static final Log LOG = LogFactory.getLog(JobManagerImpl.class);

    private static final JobRecord NULL_JOB = new JobRecord();

    private PhoneContext m_phoneContext;

    private JobQueue m_jobQueue;

    private boolean m_stop;

    private JobContext m_jobContext;

    private Object m_runMonitor = new Object();

    public void stopProcessingJobRecords() {
        m_stop = true;
        finishProcessingJobs();
    }

    public void finishProcessingJobs() {
        m_jobQueue.addJob(NULL_JOB);
        try {
            join();
        } catch (InterruptedException notFatal) {
            LOG.error("Nonfatal exception waiting for job manager to shut down", notFatal);
        }
    }

    public void run() {
        synchronized (m_runMonitor) {
            m_runMonitor.notify();
        }
        while (!m_stop) {
            // runtime exceptions will happily break loop
            JobRecord job = m_jobQueue.removeJob();
            if (job == NULL_JOB) {
                break;
            }
            runJob(job);
        }
    }

    void runJob(JobRecord job) {
        Phone[] phones = job.getPhones();
        for (int i = 0; i < phones.length && !m_stop; i++) {
            Phone phone = phones[i];
            Serializable jobId = m_jobContext.schedule("Projection for phone "
                    + phone.getSerialNumber());
            try {
                m_jobContext.start(jobId);
                switch (job.getType()) {
                case JobRecord.TYPE_PROJECTION:
                    phone.generateProfiles();
                    phone.restart();
                    break;

                case JobRecord.TYPE_DEVICE_RESTART:
                    phone.restart();
                    break;
                default:
                    break;
                }
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

    /**
     * As soon as job queue is set, jobs can be added and
     */
    public void setJobQueue(JobQueue jobQueue) {
        m_jobQueue = jobQueue;
        synchronized (m_runMonitor) {
            start();
            try {
                m_runMonitor.wait();
            } catch (InterruptedException ie) {
                throw new RuntimeException(
                        "Could not wait until job manager entered running state", ie);
            }
        }
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setJobContext(JobContext jobContext) {
        m_jobContext = jobContext;
    }
}
