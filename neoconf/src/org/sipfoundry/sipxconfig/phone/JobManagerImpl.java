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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Run an ansynchorous operation on a list of jobs.  
 */
public class JobManagerImpl extends Thread implements JobManager {

    private static final Log LOG = LogFactory.getLog(JobManagerImpl.class);

    private static final JobRecord NULL_JOB = new JobRecord();

    private PhoneContext m_phoneContext;

    private JobQueue m_jobQueue;

    private boolean m_stop;

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
        // JobRecord, a legacy business object, doesn't have the right fields to capture
        // all relevant data.  If phones 1 and 3 of 100 phones have and exception, only
        // one job record exception will be captured.
        job.setStatus(JobRecord.STATUS_STARTED);
        m_phoneContext.storeJob(job);
        char status = JobRecord.STATUS_COMPLETE;
        Phone[] phones = job.getPhones();
        for (int i = 0; i < phones.length && !m_stop; i++) {
            Phone phone = phones[i];
            String progress = null;
            job.setDetails("Projection for phone " + phone.getPhoneData().getSerialNumber());
            String progressSuffix = "" + (i + 1) + " of " + phones.length + " phones";
            try {
                job.setProgress("Starting " + progressSuffix);
                m_phoneContext.storeJob(job);
                switch (job.getType()) {
                case JobRecord.TYPE_PROJECTION:
                    progress = "projected " + progressSuffix;
                    phone.generateProfiles();
                    phone.restart();
                    break;

                case JobRecord.TYPE_DEVICE_RESTART:
                    progress = "restarted " + progressSuffix;
                    phone.restart();
                    break;
                default:
                    break;
                }
                job.setProgress(progress);
            } catch (RestartException re) {
                status = JobRecord.STATUS_FAILED;
                job.setExceptionMessage(re.getMessage());
            } catch (RuntimeException e) {
                status = JobRecord.STATUS_FAILED;
                job.setExceptionMessage(e.toString());
                throw e;
            }
            m_phoneContext.storeJob(job);
        }

        job.setStatus(status);
        m_phoneContext.storeJob(job);
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
}
