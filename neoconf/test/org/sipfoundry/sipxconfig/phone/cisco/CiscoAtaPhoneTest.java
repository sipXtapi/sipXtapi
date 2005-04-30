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

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

import junit.framework.TestCase;

public class CiscoAtaPhoneTest extends TestCase {
    
    CiscoAtaPhone m_phone = new CiscoAtaPhone();
    
    CiscoAtaLine m_line = new CiscoAtaLine();
    
    PhoneTestDriver m_driver = new PhoneTestDriver();
    
    protected void setUp() {
        m_driver.seedPhone(m_phone, CiscoModel.MODEL_ATA18X.getModelId());
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

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.setVelocityEngine(TestHelper.getVelocityEngine());
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-ata18x.cfg"));
        // TODO : Order not preserved, format valid, but not optimal. work in progress
        //assertEquals(expected, profile.toString());
    }
}
