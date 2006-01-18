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
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoAtaPhoneTest extends TestCase {

    CiscoAtaPhone phone;

    PhoneTestDriver tester;

    protected void setUp() {
        phone = new CiscoAtaPhone(CiscoModel.MODEL_ATA18X);
        tester = new PhoneTestDriver(phone);

        String testDir = TestUtil.getTestSourceDirectory(getClass());
        phone.setCfgfmtUtility(testDir + "/cfgfmt");
        phone.setPtagDat(testDir + "/ptag.dat");
    }

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        phone.packBitmaps();
        phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-ata18x.cfg"));
        assertNotNull(expected);
        assertEquals(expected, profile.toString());
    }
}
