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
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PolycomPhoneTest extends TestCase {
    
    PolycomPhone phone;
    
    PhoneTestDriver tester;
    
    protected void setUp() {
        phone = new PolycomPhone(PolycomModel.MODEL_600);
        tester = new PhoneTestDriver(phone, "polycom/phone.xml");        
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

    public void testLineDefaults() throws Exception {
        Setting settings = tester.line.getSettings();
        Setting address = settings.getSetting("reg/server/1/address");
        assertEquals("registrar.sipfoundry.org", address.getValue());
    }

    public void testLineDefaultsNoUser() throws Exception {
        Line secondLine = phone.createLine();
        phone.addLine(secondLine);
        Setting settings = secondLine.getSettings();        
        Setting userId = settings.getSetting("reg/auth.userId");
        assertEquals(Setting.NULL_VALUE, userId.getValue());
    }
}


