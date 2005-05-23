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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class GrandstreamBtPhoneTest extends TestCase {
    
    GrandstreamBtPhone m_phone = new GrandstreamBtPhone();
    
    GrandstreamBtLine m_line = new GrandstreamBtLine();
    
    PhoneTestDriver m_driver = new PhoneTestDriver();
    
    protected void setUp() {
        m_driver.seedPhone(m_phone, GrandstreamModel.MODEL_BUDGETONE.getModelId());
        m_phone.setTftpRoot(TestHelper.getTestDirectory());
        m_driver.seedLine(m_line, GrandstreamBtLine.FACTORY_ID);
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
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-gsbt.cfg"));
        assertEquals(expected, profile.toString());
    }
}
