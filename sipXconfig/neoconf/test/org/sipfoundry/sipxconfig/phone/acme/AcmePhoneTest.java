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
package org.sipfoundry.sipxconfig.phone.acme;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class AcmePhoneTest extends TestCase {

    public void testGenerateTypicalProfile() throws Exception {
        AcmePhone phone = new AcmePhone();
        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
                
        phone.generateProfiles();
        
        InputStream expectedProfile = getClass().getResourceAsStream("expected-config");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        
        assertEquals(expected, location.toString());
    }
}
