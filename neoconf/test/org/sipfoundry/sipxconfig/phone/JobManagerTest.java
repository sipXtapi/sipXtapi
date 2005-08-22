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
import org.sipfoundry.sipxconfig.job.JobContext;

public class JobManagerTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        Integer jobId = new Integer(4);

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Projection for phone 000000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.generateProfiles();
        phoneControl.setVoidCallable(1);
        phone.restart();
        phoneControl.setVoidCallable(1);
        phoneControl.replay();

        JobRecord job = new JobRecord();
        job.setPhones(new Phone[] {
            phone
        });

        MockControl phoneContextControl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextControl.getMock();
        job.setType(JobRecord.TYPE_PROJECTION);
        phoneContextControl.replay();

        JobManagerImpl mgr = new JobManagerImpl();
        mgr.setPhoneContext(phoneContext);
        mgr.setJobContext(jobContext);
        mgr.runJob(job);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextControl.verify();
    }

    public void testRestartException() throws Exception {
        Integer jobId = new Integer(4);

        RestartException re = new RestartException("xxx");

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Projection for phone 000000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.failure(jobId, null, re);
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.restart();
        phoneControl.setThrowable(re);
        phoneControl.replay();

        JobRecord job = new JobRecord();
        job.setPhones(new Phone[] {
            phone
        });

        MockControl phoneContextControl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextControl.getMock();
        job.setType(JobRecord.TYPE_DEVICE_RESTART);
        phoneContextControl.replay();

        JobManagerImpl mgr = new JobManagerImpl();
        mgr.setPhoneContext(phoneContext);
        mgr.setJobContext(jobContext);
        mgr.runJob(job);

        phoneControl.verify();
        phoneContextControl.verify();
        jobContextCtrl.verify();
    }

    public void testStartStop() throws Exception {
        JobManagerImpl mgr = new JobManagerImpl();
        JobQueue queue = new JobQueue();
        mgr.setJobQueue(queue);
        mgr.finishProcessingJobs();
        assertTrue(queue.isEmpty());
    }
}
