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
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class CiscoAtaPhoneTest extends TestCase {
    private CiscoModel m_model;
    protected void setUp() {
        m_model = new CiscoModel();
        m_model.setBeanId(CiscoAtaPhone.BEAN_ID);
    }

    public void testAtaProfile() throws Exception {        
        m_model.setCfgPrefix("ata");
        m_model.setModelId("cisco18x");
        m_model.setMaxLineCount(2);
        runPhoneTest("expected-ata18x.cfg");
    }

    public void testNonAtaProfile() throws Exception {
        m_model.setCfgPrefix("gk");
        m_model.setModelId("cisco7912");
        m_model.setMaxLineCount(1);
        runPhoneTest("expected-7912.cfg");
    }
    
    void runPhoneTest(String expectedFile) throws Exception {
        CiscoAtaPhone phone = new CiscoAtaPhone();
        phone.setModel(m_model);
        PhoneTestDriver.supplyTestData(phone);
        String testDir = TestUtil.getTestSourceDirectory(getClass());
        phone.setCfgfmtUtility(testDir + "/cfgfmt");
        phone.setPtagDat(testDir + "/ptag.dat");
        StringWriter profile = new StringWriter();
        phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                expectedFile));
        assertNotNull(expected);
        assertEquals(expected, profile.toString());     
    }
}
