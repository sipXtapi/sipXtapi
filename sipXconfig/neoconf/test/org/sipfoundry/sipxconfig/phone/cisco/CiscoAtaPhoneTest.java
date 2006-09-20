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

    public void testAtaProfile() throws Exception {        
        runPhoneTest(CiscoModel.MODEL_ATA18X, "expected-ata18x.cfg");
    }

    public void test7912Profile() throws Exception {
        runPhoneTest(CiscoModel.MODEL_7912, "expected-7912.cfg");
    }
    
    public void test7905Profile() throws Exception {
        runPhoneTest(CiscoModel.MODEL_7905, "expected-7912.cfg");
    }

    void runPhoneTest(CiscoModel model, String expectedFile) throws Exception {
        CiscoAtaPhone phone = new CiscoAtaPhone(model);
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
