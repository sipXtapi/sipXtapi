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

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        
        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        
        // Display name because value comes from LineSettings now, not User object
        // kphone does not store Display name directory, but uses it as part of URI
        // would need a URI parser to get it back.
        assertEquals(expected, actualWriter.toString());
    }
}
