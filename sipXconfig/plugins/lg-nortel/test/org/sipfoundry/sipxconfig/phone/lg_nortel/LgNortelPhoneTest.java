/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class LgNortelPhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new LgNortelModel()));
    }

    public void testGetFileName() throws Exception {
        LgNortelPhone phone = new LgNortelPhone();
        phone.setSerialNumber("0011aabb4455");
        phone.setTftpRoot("abc");
        assertEquals("abc/0011AABB4455.cfg", phone.getPhoneFilename());
    }

    //FIXME: disable temporary during development
    public void testGenerateTypicalProfile() throws Exception {
        LgNortelPhone phone = new LgNortelPhone(new LgNortelModel());

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(phone.getPhoneTemplate(), actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = actualWriter.toString();
        System.err.println(actual);

        String expectedLines[] = StringUtils.split(expected, "\n");
        String actualLines[] = StringUtils.split(actual, "\n");

        int len = Math.min(actualLines.length, expectedLines.length);
        for (int i = 0; i < len; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
        assertEquals(expectedLines.length, actualLines.length);
    }
}
