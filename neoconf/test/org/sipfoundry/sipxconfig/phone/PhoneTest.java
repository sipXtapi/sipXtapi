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
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.io.IOException;

import junit.framework.TestCase;

public class PhoneTest extends TestCase {

    public void testSetSerialNumber() {
        Phone phone = new Phone();
        phone.setSerialNumber("123456789012");
        assertEquals("123456789012", phone.getSerialNumber());
        phone.setSerialNumber("1234 5678 9012");
        assertEquals("123456789012", phone.getSerialNumber());
        phone.setSerialNumber("12:34:56:78:90:12");
        assertEquals("123456789012", phone.getSerialNumber());
        phone.setSerialNumber("AABBCCDDEEFF");
        assertEquals("aabbccddeeff", phone.getSerialNumber());
        phone.setSerialNumber("totallybogus");
        assertEquals("totallybogus", phone.getSerialNumber());
    }

    public void testGenerateAndRemoveProfiles() {
        Phone phone = new Phone() {
            private String m_name;

            public String getPhoneFilename() {
                if (m_name != null) {
                    return m_name;
                }
                try {
                    File f = File.createTempFile("phone", "cfg");
                    f.delete();
                    m_name = f.getPath();
                    return m_name;
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        };

        String phoneFilename = phone.getPhoneFilename();
        System.err.println(phoneFilename);
        File profile = new File(phoneFilename);
        phone.generateProfiles();
        assertTrue(profile.exists());
        phone.removeProfiles();
        assertFalse(profile.exists());
    }
}
