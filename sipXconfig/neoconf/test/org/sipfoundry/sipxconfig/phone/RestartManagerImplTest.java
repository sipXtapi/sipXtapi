/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.job.JobContext;

public class RestartManagerImplTest extends TestCase {
    public void testGenerateProfiles() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Restarting phone 000000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.restart();
        phoneControl.replay();

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.setReturnValue(phone);
        phoneContextCtrl.replay();

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

        rm.restart(phoneId);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
    }

    public void testRestartException() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        RestartException re = new RestartException("xxx");

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Restarting phone 000000000000");
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

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.setReturnValue(phone);
        phoneContextCtrl.replay();

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

        rm.restart(phoneId);

        phoneControl.verify();
        jobContextCtrl.verify();
    }

    public void testThrottle() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        for (int i = 0; i < 2; i++) {
            jobContext.schedule("Restarting phone 110000000000");
            jobContextCtrl.setReturnValue(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
            jobContext.schedule("Restarting phone 120000000000");
            jobContextCtrl.setReturnValue(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
        }
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        for (int i = 0; i < 2; i++) {
            phoneControl.expectAndReturn(phone.getSerialNumber(), "110000000000");
            phone.restart();
            phoneControl.expectAndReturn(phone.getSerialNumber(), "120000000000");
            phone.restart();
        }
        phoneControl.replay();

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        for (int i = 0; i < 2; i++) {
            phoneContext.loadPhone(ids[0]);
            phoneContextCtrl.setReturnValue(phone);
            phoneContext.loadPhone(ids[1]);
            phoneContextCtrl.setReturnValue(phone);
        }
        phoneContextCtrl.replay();

        final int throttle = 50;

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

        rm.setThrottleInterval(0);

        long before = System.currentTimeMillis();
        rm.restart(Arrays.asList(ids));
        long duration = System.currentTimeMillis() - before;
        assertTrue(duration < 2 * throttle);

        rm.setThrottleInterval(throttle);

        before = System.currentTimeMillis();
        rm.restart(Arrays.asList(ids));
        duration = System.currentTimeMillis() - before;
        assertTrue(duration >= 2 * throttle);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
    }
}
