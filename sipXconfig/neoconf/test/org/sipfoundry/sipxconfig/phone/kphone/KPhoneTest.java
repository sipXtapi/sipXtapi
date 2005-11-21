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
package org.sipfoundry.sipxconfig.phone.kphone;

import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class KPhoneTest extends TestCase {

    public void testGenerateTypicalProfile() throws Exception {
        KPhone phone = new KPhone();

        // call this method purely for the side effect of modifying the phone, and
        // discard the resulting PhoneTestDriver object
        new PhoneTestDriver(phone);
        
        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, actualWriter.toString());
    }
}
