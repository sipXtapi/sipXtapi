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
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ClearonePhoneTest extends TestCase {
    public void testFactoryRegistered() {
//        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
//                PhoneContext.CONTEXT_BEAN_NAME);
//        assertNotNull(pc.newPhone(new ClearoneModel()));
    }

    public void testGetFileName() throws Exception {
        ClearonePhone phone = new ClearonePhone();
        phone.setSerialNumber("001122334455");
        phone.setTftpRoot("abc");
        assertEquals("abc/C1MAXIP_001122334455.txt", phone.getPhoneFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        ClearonePhone phone = new ClearonePhone(new ClearoneModel());

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("C1MAXIP.txt");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        String actual = actualWriter.toString();
        
        expected = expected.replaceAll(" +", " ");
        actual = actual.replaceAll(" +", " ");
        assertEquals(expected, actual);

    }
}
