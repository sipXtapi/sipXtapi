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
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoIpPhoneTest extends TestCase {
    
    CiscoIpPhone m_phone = new CiscoIpPhone();
    
    CiscoIpLine m_line = new CiscoIpLine();
    
    PhoneTestDriver m_driver = new PhoneTestDriver();
    
    protected void setUp() {
        m_driver.seedPhone(m_phone, CiscoModel.MODEL_7960.getModelId());
        m_phone.setTftpRoot(TestHelper.getTestDirectory());
        m_driver.seedLine(m_line, CiscoIpLine.FACTORY_ID);
        m_driver.replay();
    }
    
    protected void tearDown() {
        m_driver.verify();
    }
    
    public void testGetSettings() {
        m_phone.getSettings();
    }

    public void testGenerate7960Profiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.setVelocityEngine(TestHelper.getVelocityEngine());
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-7960.cfg"));
        assertEquals(expected, profile.toString());
    }
    
    public void testGenerateAtaProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.setVelocityEngine(TestHelper.getVelocityEngine());
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-7960.cfg"));
        assertEquals(expected, profile.toString());
    }
}


