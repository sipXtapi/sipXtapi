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
package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PolycomPhoneTest extends TestCase {
    
    PolycomPhone phone;
    
    PolycomLine line;

    PhoneTestDriver tester;
    
    protected void setUp() {
        phone = new PolycomPhone();
        line = new PolycomLine();
        tester = new PhoneTestDriver(phone, PolycomModel.MODEL_600.getName(), line, 
                PolycomLine.FACTORY_ID);
    }
            
    public void testDefaults() throws Exception {
        Setting settings = phone.getSettings();
        Setting address = settings.getSetting("voIpProt/SIP.outboundProxy/address");
        assertEquals("proxy.sipfoundry.org", address.getValue());
    }

    public void testGenerateProfiles() throws Exception {
        phone.setVelocityEngine(TestHelper.getVelocityEngine());
        phone.generateProfiles();
        
        // content of profiles is tested in individual base classes of ConfigurationTemplate
    }
    
    public void testRestartFailureNoLine() throws Exception {
        phone.getLines().clear();
        try {
            phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }
    
    public void testRestart() throws Exception {
        phone.restart();
        tester.sipControl.verify();
    }
}


