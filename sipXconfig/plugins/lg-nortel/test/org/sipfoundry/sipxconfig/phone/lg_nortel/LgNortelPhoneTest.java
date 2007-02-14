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
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
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
        assertEquals("abc/0011AABB4455", phone.getPhoneFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        LgNortelModel lgNortelModel = new LgNortelModel();
        lgNortelModel.setMaxLineCount(4); // we are testing 2 lines
        LgNortelPhone phone = new LgNortelPhone(lgNortelModel);

        User u1 = new User();
        u1.setUserName("juser");
        u1.setFirstName("Joe");
        u1.setLastName("User");
        u1.setSipPassword("1234");

        User u2 = new User();
        u2.setUserName("buser");
        u2.setSipPassword("abcdef");

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(new User[] {
            u1, u2
        }));

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(phone.getPhoneTemplate(), actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("mac.cfg");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = actualWriter.toString();

        String expectedLines[] = StringUtils.split(expected, "\n");
        String actualLines[] = StringUtils.split(actual, "\n");

        int len = Math.min(actualLines.length, expectedLines.length);
        for (int i = 0; i < len; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
        assertEquals(expectedLines.length, actualLines.length);
    }
}
