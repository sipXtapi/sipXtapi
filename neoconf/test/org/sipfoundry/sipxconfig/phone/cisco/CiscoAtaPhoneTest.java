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

import java.io.IOException;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class CiscoAtaPhoneTest extends TestCase {
    
    CiscoAtaPhone phone;
    
    CiscoAtaLine line;

    PhoneTestDriver tester;
    
    protected void setUp() throws IOException {
        phone = new CiscoAtaPhone();
        line = new CiscoAtaLine();
        tester = new PhoneTestDriver(phone, CiscoModel.MODEL_ATA18X.getModelId(), line, 
                CiscoAtaLine.FACTORY_ID);

        String testDir = TestUtil.getTestSourceDirectory(getClass());
        phone.setCfgfmtUtility(testDir + "/cfgfmt");
        phone.setPtagDat(testDir + "/ptag.dat");
    }
    
    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-ata18x.cfg"));
        assertNotNull(expected);
        // TODO : Order not preserved, format valid, but not optimal. work in progress
        //assertEquals(expected, profile.toString());
    }
}
