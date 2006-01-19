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
}
