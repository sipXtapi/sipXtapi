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

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;


public class JobManagerTest extends TestCase {
        
    public void testGenerateProfiles() throws Exception {
        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.generateProfiles();        
        phoneControl.setVoidCallable(1);
        phone.restart();
        phoneControl.setVoidCallable(1);
        phoneControl.replay();

        JobRecord job = new JobRecord();
        job.setPhones(new Phone[] {phone});        

        MockControl phoneContextControl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextControl.getMock();
        job.setType(JobRecord.TYPE_PROJECTION);
        phoneContext.storeJob(job);
        phoneContextControl.setVoidCallable(4);
        phoneContextControl.replay();                               

        JobManagerImpl mgr = new JobManagerImpl();
        mgr.setPhoneContext(phoneContext);        
        mgr.runJob(job);
        
        assertNotNull(job.getProgress());
        assertNotNull(job.getDetails());
        assertNull(job.getExceptionMessage());
        
        phoneControl.verify();
        phoneContextControl.verify();        
    }
    
    public void testRestartException() throws Exception {
        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.restart();
        phoneControl.setVoidCallable(1);
        phoneControl.replay();

        JobRecord job = new JobRecord();
        job.setPhones(new Phone[] {phone});        

        MockControl phoneContextControl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextControl.getMock();
        job.setType(JobRecord.TYPE_DEVICE_RESTART);
        phoneContext.storeJob(job);
        phoneContextControl.setVoidCallable(4);
        phoneContextControl.replay();                               

        JobManagerImpl mgr = new JobManagerImpl();
        mgr.setPhoneContext(phoneContext);        
        mgr.runJob(job);
        
        assertNotNull(job.getProgress());
        assertNotNull(job.getDetails());
        assertNull(job.getExceptionMessage());
        
        phoneControl.verify();
        phoneContextControl.verify();        
    }
    
    public void testStartStop() throws Exception {        
        JobManagerImpl mgr = new JobManagerImpl();
        JobQueue queue = new JobQueue();        
        mgr.setJobQueue(queue);
        mgr.finishProcessingJobs();
        assertTrue(queue.isEmpty());
    }
}
