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
package org.sipfoundry.sipxconfig.phone.hitachi;

import java.io.InputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class HitachiPhoneTest extends TestCase {
    public void testFactoryRegistered() {
        // FIXME: TestHelper is not accesible here - need to find a way of loading application
        // context
        // PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
        // PhoneContext.CONTEXT_BEAN_NAME);
        // assertNotNull(pc.newPhone(HitachiModel.MODEL_3000));
        // assertNotNull(pc.newPhone(HitachiModel.MODEL_5000));
        // assertNotNull(pc.newPhone(HitachiModel.Model_5000A));
    }

    public void testGetFileName() throws Exception {
        HitachiPhone phone = new HitachiPhone();
        phone.setSerialNumber("001122334455");
        assertEquals("334455user.ini", phone.getPhoneFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        HitachiPhone phone = new HitachiPhone(new HitachiModel("hitachi5000"));

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
                
        phone.generateFile(phone.getPhoneTemplate(), phone.getPhoneTemplate());
        InputStream expectedProfile = getClass().getResourceAsStream("test.user.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, location.toString());
    }
}
