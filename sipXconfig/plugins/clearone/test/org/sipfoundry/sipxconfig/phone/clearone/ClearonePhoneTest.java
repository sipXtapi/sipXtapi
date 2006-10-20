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
package org.sipfoundry.sipxconfig.phone.clearone;

import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ClearonePhoneTest extends TestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new ClearoneModel()));
    }

    public void testGetFileName() throws Exception {
        ClearonePhone phone = new ClearonePhone();
        phone.setSerialNumber("0011aabb4455");
        phone.setTftpRoot("abc");
        assertEquals("abc/C1MAXIP_0011AABB4455.txt", phone.getPhoneFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        ClearonePhone phone = new ClearonePhone(new ClearoneModel());

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(phone.getPhoneTemplate(), actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("C1MAXIP.txt");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = actualWriter.toString();
        
        String expectedLines[] = StringUtils.split(expected.replaceAll(" +", " "), "\n");
        String actualLines[] = StringUtils.split(actual.replaceAll(" +", " "), "\n");                

        assertEquals(expectedLines.length, actualLines.length);
        for (int i = 0; i < actualLines.length; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
    }
    
    public void testGenerateTypicalDialplan() throws Exception {
        ClearonePhone phone = new ClearonePhone(new ClearoneModel());

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(phone.getDialplanTemplate(), actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("c1dialplan.txt");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = actualWriter.toString();
        
        String expectedLines[] = StringUtils.split(expected.replaceAll(" +", " "), "\n");
        String actualLines[] = StringUtils.split(actual.replaceAll(" +", " "), "\n");                

        assertEquals(expectedLines.length, actualLines.length);
        for (int i = 0; i < actualLines.length; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
    }
    
}
