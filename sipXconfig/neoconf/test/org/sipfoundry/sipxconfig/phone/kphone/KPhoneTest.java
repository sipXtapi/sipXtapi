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

import java.io.FileReader;
import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneData;

public class KPhoneTest extends TestCase {

    public void testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        assertTrue(pc.getPhoneFactoryIds().containsKey("kphone"));        
    }

    public void testGenerateProfile() throws Exception {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        
        // Defining FACTORY_ID is not nec, just good practice
        KPhone kphone = (KPhone) pc.newPhone(KPhone.FACTORY_ID);
        
        // All phones in system have a unique id, this will be important for
        // selecting which profile to download 
        kphone.getPhoneData().setSerialNumber("000000000000");
        
        // method to test
        kphone.generateProfiles();
        
        // Compare profile output with expected output
        FileReader actualProfile = new FileReader(kphone.getProfileFileName());
        String actual = IOUtils.toString(actualProfile);
        actualProfile.close();
        
        // test output file is a copy of the basic template and located in same directory
        // as this java source file
        InputStream expectedProfile = getClass().getResourceAsStream("empty-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        
        assertEquals(expected, actual);
    }
    
    public void testDefaultProfile() throws Exception {
        // Mock objects let you test calls are made to other classes in proper
        // order, and convienent for returning dummy data
        MockControl pcControl = MockControl.createNiceControl(PhoneContext.class);
        PhoneContext pc = (PhoneContext) pcControl.getMock();
        pcControl.expectAndReturn(pc.getSystemDirectory(), TestHelper.getSysDirProperties().get("sysdir.etc"), 
                MockControl.ZERO_OR_MORE);
        // mock creating lines from spring file
        pcControl.expectAndReturn(pc.newLine(KPhoneLine.FACTORY_ID), new KPhoneLine());
        // used when building URLs
        pcControl.expectAndReturn(pc.getDnsDomain(), "sipfoundry.org");
        pcControl.replay();
        
        // Seed phone with typical settings
        KPhone kphone = new KPhone();        
        kphone.setPhoneData(new PhoneData());
        kphone.setPhoneContext(pc);
        kphone.getPhoneData().setSerialNumber("000000000000");
        kphone.setVelocityEngine(TestHelper.getVelocityEngine());
        LineData lineData = new LineData();
        User user = new User();
        user.setFirstName("Joe");
        user.setLastName("Smith");
        lineData.setUser(user);
        user.setDisplayId("jsmith");
        kphone.addLine(kphone.createLine(lineData));

        // generate profile in memory buffer to isolate test logic 
        StringWriter actualWriter = new StringWriter();
        kphone.generateProfile(actualWriter);        
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, actualWriter.toString());
        
        pcControl.verify();
    }    
}
