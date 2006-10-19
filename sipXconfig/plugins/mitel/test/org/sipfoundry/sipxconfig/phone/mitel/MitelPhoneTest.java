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
package org.sipfoundry.sipxconfig.phone.mitel;

import java.io.InputStream;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class MitelPhoneTest extends XMLTestCase {
    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new MitelModel()));
    }

    public void testGetFileName() throws Exception {
        MitelPhone phone = new MitelPhone();
        phone.setSerialNumber("001122334455");
        phone.setTftpRoot("abc");
        assertEquals("abc/mn_001122334455.txt", phone.getPhoneFilename());
    }

    public void testGenerateTypicalProfile() throws Exception {
        MitelPhone phone = new MitelPhone(new MitelModel());

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(phone);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(actualWriter);
        InputStream expectedProfile = getClass().getResourceAsStream("mn.txt");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        
        String actual = actualWriter.toString();
        System.err.println(actual);

        XMLUnit.setIgnoreWhitespace(true);
        assertXMLEqual(expected, actual);
    }
}
