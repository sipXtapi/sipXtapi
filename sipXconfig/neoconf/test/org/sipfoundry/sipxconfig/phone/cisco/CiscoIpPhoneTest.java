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

public class CiscoIpPhoneTest extends TestCase {

    private CiscoIpPhone m_phone;

    protected void setUp() {
        m_phone = new CiscoIpPhone(CiscoModel.MODEL_7960);
        new PhoneTestDriver(m_phone);
    }

    public void testGetSettings() {
        m_phone.getSettings();
    }

    public void testGenerate7960Profiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream(
                "expected-7960.cfg"));
        assertEquals(expected, profile.toString());
    }
}
