/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.kphone;

import java.io.InputStream;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

import junit.framework.TestCase;

public class KPhoneTestDb extends TestCase {

    public void testGenerateEmptyProfile() throws Exception {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);

        KPhone kphone = (KPhone) pc.newPhone(KPhone.MODEL);

        // All phones in system have a unique id, this will be important for
        // selecting which profile to download 
        kphone.setSerialNumber("000000000000");

        // method to test
        StringWriter actual = new StringWriter();
        kphone.generateProfile(actual);

        // test output file is a copy of the basic template and located in same directory
        // as this java source file
        InputStream expectedProfile = getClass().getResourceAsStream("empty-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, actual.toString());
    }

}
