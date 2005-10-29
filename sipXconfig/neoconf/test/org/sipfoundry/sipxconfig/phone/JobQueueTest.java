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
import org.sipfoundry.sipxconfig.job.JobManager;

public class JobQueueTest extends TestCase {

    private JobQueue m_queue;

    protected void setUp() {
        m_queue = new JobQueue();
    }

    public void testGenerateProfiles() throws Exception {
        Integer jobId = new Integer(4);

        MockControl jobContextCtrl = MockControl.createStrictControl(JobManager.class);
        JobManager jobContext = (JobManager) jobContextCtrl.getMock();
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

        job.setType(JobRecord.TYPE_PROJECTION);

        m_queue.setJobContext(jobContext);
        m_queue.addJob(job);
        m_queue.yieldTillEmpty();

        jobContextCtrl.verify();
        phoneControl.verify();
    }

    public void testRestartException() throws Exception {
        Integer jobId = new Integer(4);

        RestartException re = new RestartException("xxx");

        MockControl jobContextCtrl = MockControl.createStrictControl(JobManager.class);
        JobManager jobContext = (JobManager) jobContextCtrl.getMock();
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

        job.setType(JobRecord.TYPE_DEVICE_RESTART);

        m_queue.setJobContext(jobContext);
        m_queue.addJob(job);
        m_queue.yieldTillEmpty();

        phoneControl.verify();
        jobContextCtrl.verify();
    }

    public void testStartStop() throws Exception {
        JobQueue queue = new JobQueue();
        assertTrue(queue.isEmpty());
    }
}
