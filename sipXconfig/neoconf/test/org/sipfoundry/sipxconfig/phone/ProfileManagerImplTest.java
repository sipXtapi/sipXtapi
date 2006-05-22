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
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.job.JobContext;

public class ProfileManagerImplTest extends TestCase {
    
    public void testNewPhone() {
        new Phone();
    }

    public void DISABLED_testGenerateProfilesAndRestart() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Projection for phone 110000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContext.schedule("Projection for phone 120000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "110000000000");
        phone.generateProfiles();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "120000000000");
        phone.generateProfiles();
        phoneControl.replay();

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContext.loadPhone(ids[0]);
        phoneContextCtrl.setReturnValue(phone);
        phoneContext.loadPhone(ids[1]);
        phoneContextCtrl.setReturnValue(phone);
        phoneContextCtrl.replay();

        MockControl restartManagerCtrl = MockControl.createControl(RestartManager.class);
        RestartManager restartManager = (RestartManager) restartManagerCtrl.getMock();
        restartManager.restart(ids[0]);
        restartManager.restart(ids[1]);
        restartManagerCtrl.replay();

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setPhoneContext(phoneContext);
        pm.setRestartManager(restartManager);

        pm.generateProfilesAndRestart(Arrays.asList(ids));

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
        restartManagerCtrl.verify();
    }

    public void DISABLED_testGenerateProfileAndRestart() {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        MockControl jobContextCtrl = MockControl.createStrictControl(JobContext.class);
        JobContext jobContext = (JobContext) jobContextCtrl.getMock();
        jobContext.schedule("Projection for phone 110000000000");
        jobContextCtrl.setReturnValue(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = new Phone();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "110000000000");
        phone.generateProfiles();
        phoneControl.replay();

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.setReturnValue(phone);
        phoneContextCtrl.replay();

        MockControl restartManagerCtrl = MockControl.createControl(RestartManager.class);
        RestartManager restartManager = (RestartManager) restartManagerCtrl.getMock();
        restartManager.restart(phoneId);
        restartManagerCtrl.replay();

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setPhoneContext(phoneContext);
        pm.setRestartManager(restartManager);

        pm.generateProfileAndRestart(phoneId);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
        restartManagerCtrl.verify();
    }

}
