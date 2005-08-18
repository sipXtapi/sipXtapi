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
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class GrandstreamPhoneTest extends TestCase {
    
    GrandstreamPhone phone;
    
    PhoneTestDriver tester;
    
    protected void setUp() {
        phone = new GrandstreamPhone(GrandstreamModel.MODEL_BUDGETONE);
        tester = new PhoneTestDriver(phone, "grandstream/phone.xml");
    }

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-gsbt.cfg"));
        assertEquals(expected, profile.toString());
    }
}
