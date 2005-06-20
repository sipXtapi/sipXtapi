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
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class KPhoneTest extends TestCase {

    public void testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        assertTrue(pc.getPhoneFactoryIds().containsKey("kphone"));        
    }

    // TODO: this test loads database - needs to be moved to TestDb or rewritten
    
    public void _testGenerateEmptyProfile() throws Exception {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        
        // Defining FACTORY_ID is not nec, just good practice
        KPhone kphone = (KPhone) pc.newPhone(KPhone.FACTORY_ID);
        
        // All phones in system have a unique id, this will be important for
        // selecting which profile to download 
        kphone.getPhoneData().setSerialNumber("000000000000");
        
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
    
    public void testGenerateTypicalProfile() throws Exception {
        KPhone phone = new KPhone();
        KPhoneLine line = new KPhoneLine();
        PhoneTestDriver tester = new PhoneTestDriver(phone, KPhone.FACTORY_ID, line, 
                KPhoneLine.FACTORY_ID);

        StringWriter actualWriter = new StringWriter();
        phone.generateProfile(actualWriter);        
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, actualWriter.toString());
    }    
}
