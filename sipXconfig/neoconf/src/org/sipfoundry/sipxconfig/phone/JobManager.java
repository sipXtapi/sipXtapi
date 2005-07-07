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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Run an ansynchorous operation on a list of jobs.  
 */
public class JobManager extends Thread {
    
    private static Log s_log = LogFactory.getLog(JobManager.class);

    private Iterator m_phones;
    
    private PhoneJob m_job;
    
    private volatile boolean m_stop;
    
    private BatchPhoneException[] m_exceptions;
    
    public void stopJob() {
        m_stop = true;
    }
    
    public void startJob(Iterator phones, PhoneJob job) {
        m_phones = phones;
        m_job = job;
        start();
    }
    
    public void run() {
        List exceptions = new ArrayList();
        while (!m_stop && m_phones.hasNext()) {
            Phone phone = (Phone) m_phones.next();
            try {
                m_job.operate(phone);
            } catch (Throwable t) {
                exceptions.add(new BatchPhoneException(phone, t));
                // TODO: XCF-225
                s_log.error("could not complete job on phone " 
                        + phone.getPhoneData().getDisplayLabel(), t);
            }
        }
        
        m_exceptions = (BatchPhoneException[]) exceptions.toArray(new BatchPhoneException[0]);        
    }
    
    public BatchPhoneException[] getExceptions() {
        return m_exceptions;
    }    
    
    /**
     * Generate profile on phones in background
     */
    public void generateProfiles(Iterator phones) {
        PhoneJob job = new PhoneJob() {
            public void operate(Phone phone) {
                phone.generateProfiles();
            }
        };
        startJob(phones, job);        
    }
    
    /**
     * Restart phones in background
     */
    public void restart(Iterator phones) {
        PhoneJob job = new PhoneJob() {
            public void operate(Phone phone) {
                phone.restart();
            }
        };
        startJob(phones, job);        
    }
    
    /**
     * Generate profiles and restart phones that successfully generated profiles
     */
    public void generateProfilesAndRestart(Iterator phones) {
        PhoneJob job = new PhoneJob() {
            public void operate(Phone phone) {
                phone.generateProfiles();
                phone.restart();
            }
        };
        startJob(phones, job);        
    }
}
