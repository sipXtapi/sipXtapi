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


public class BatchPhoneJobTest extends TestCase {
    
    private int m_count;
    
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
    
    public void testCustomJob() throws Exception {
        MockControl control = MockControl.createStrictControl(Phone.class);
        Phone phone = (Phone) control.getMock(); 
        control.expectAndReturn(phone.getDisplayLabel(), "", 1001);
        control.replay();
    
        PhoneJob job = new PhoneJob() {
            public void operate(Phone phone) {
                phone.getDisplayLabel();
                if ((m_count++ % 10) == 0) {
                    throw new RuntimeException("Mod 10 test");
                }
            }
        };

        JobManager jmgr = new JobManager();
        jmgr.startJob(createPhones(phone, 1001), job);        
        jmgr.join();
        
        control.verify();
        assertEquals(1001, m_count);
        assertEquals(101, jmgr.getExceptions().size());
    }
}
