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

import java.util.Arrays;
import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.MockControl;


public class JobManagerTest extends TestCase {
    
    public void testRestart() throws Exception {
        MockControl control = MockControl.createStrictControl(Phone.class);
        Phone phone = (Phone) control.getMock(); 
        phone.restart();
        control.setVoidCallable(1001);
        control.replay();
        
        JobManager jmgr = new JobManager();
        jmgr.restart(createPhones(phone, 1001));        
        jmgr.join();
        
        control.verify();
    }
    
    private Iterator createPhones(Phone phone, int size) {
        Phone[] phones = new Phone[size];
        Arrays.fill(phones, phone);
        return Arrays.asList(phones).iterator();
    }
    
    public void testGenerateProfiles() throws Exception {
        MockControl control = MockControl.createStrictControl(Phone.class);
        Phone phone = (Phone) control.getMock(); 
        phone.generateProfiles();
        control.setVoidCallable(1001);
        control.replay();
        
        JobManager jmgr = new JobManager();
        jmgr.generateProfiles(createPhones(phone, 1001));        
        jmgr.join();
        
        control.verify();
    }
    
    /** 
     * Make sure phone doesn't try to restart if it didn't generate profiles
     */
    public void testGenerateProfilesAndRestart() throws Exception {
        MockControl control = MockControl.createStrictControl(Phone.class);
        Phone phone = (Phone) control.getMock(); 
        phone.generateProfiles();
        phone.restart();
        phone.generateProfiles();
        control.setThrowable(new RuntimeException("MOCK TEST"), 1);
        control.expectAndReturn(phone.getPhoneMetaData(), new PhoneMetaData());
        control.replay();
        
        JobManager jmgr = new JobManager();
        jmgr.generateProfilesAndRestart(createPhones(phone, 2));        
        jmgr.join();
        
        control.verify();
    }

    public void testCustomJob() throws Exception {
        MockControl control = MockControl.createStrictControl(Phone.class);
        Phone phone = (Phone) control.getMock(); 
        control.expectAndReturn(phone.getPhoneMetaData(), new PhoneMetaData(), 5);
        control.replay();
        
        MockControl jobControl = MockControl.createStrictControl(PhoneJob.class);
        PhoneJob job = (PhoneJob) jobControl.getMock();
        job.operate(phone);
        jobControl.setThrowable(new RuntimeException("MOCK TEST"), 5);
        jobControl.setVoidCallable(5);
        jobControl.replay();

        JobManager jmgr = new JobManager();
        jmgr.startJob(createPhones(phone, 10), job);        
        jmgr.join();
        
        jobControl.verify();
        control.verify();
        assertEquals(5, jmgr.getExceptions().length);
    }
}
