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

/**
 * Run an ansynchorous operation on a list of jobs.  
 */
public class JobManager extends Thread {
    
    private Iterator m_phones;
    
    private PhoneJob m_job;
    
    private volatile boolean m_stop;
    
    private List m_exceptions = new ArrayList();
    
    public void stopJob() {
        m_stop = true;
    }
    
    public void startJob(Iterator phones, PhoneJob job) {
        m_phones = phones;
        m_job = job;
        start();
    }
    
    public void run() {
        while (!m_stop && m_phones.hasNext()) {
            Phone phone = (Phone) m_phones.next();
            try {
                m_job.operate(phone);
            } catch (Throwable t) {
                m_exceptions.add(new BatchPhoneException(phone, t));
                // TOD: XCF-225
                t.printStackTrace();
            }
        }
    }
    
    public List getExceptions() {
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
}
