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

import java.io.IOException;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class GrandstreamPhoneTest extends TestCase {
    
    GrandstreamPhone phone;
    
    GrandstreamLine line;

    PhoneTestDriver tester;
    
    protected void setUp() throws IOException {
        phone = new GrandstreamPhone();
        line = new GrandstreamLine();
        tester = new PhoneTestDriver(phone, GrandstreamModel.MODEL_BUDGETONE.getModelId(), line, 
                GrandstreamLine.FACTORY_ID);
    }

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-gsbt.cfg"));
        assertEquals(expected, profile.toString());
    }
}
