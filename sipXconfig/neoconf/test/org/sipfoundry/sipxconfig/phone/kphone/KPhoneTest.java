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

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class KPhoneTest extends TestCase {

    public void testGenerateTypicalProfile() throws Exception {
        KPhone phone = new KPhone();

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles();
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        // Display name because value comes from LineSettings now, not User object
        // kphone does not store Display name directory, but uses it as part of URI
        // would need a URI parser to get it back.
        assertEquals(expected, location.toString());
    }

    public void testGenerateEmptyProfile() throws Exception {
        KPhone phone = new KPhone();
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.setModelFilesContext(TestHelper.getModelFilesContext());

        // All phones in system have a unique id, this will be important for
        // selecting which profile to download
        phone.setSerialNumber("000000000000");

        // method to test
        phone.generateProfiles();

        // test output file is a copy of the basic template and located in same directory
        // as this java source file
        InputStream expectedProfile = getClass().getResourceAsStream("empty-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
}
