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
import java.io.IOException;
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

/**
 * Test the KPhone 
 */
public class KPhoneTest extends TestCase {

    /**
     * Kphone is loadable from Spring Framework context file
     */
    public void testPlugin() {
        KPhone kphone = (KPhone) TestHelper.getApplicationContext().getBean(KPhone.FACTORY_ID);
        assertNotNull(kphone);
    }
    
    /**
     * Kphone is available as a selectable phone
     */
    public void testPluginAvailable() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        // Make sure it's not in production
        assertFalse(pc.getPhoneFactoryIds().containsKey(KPhone.FACTORY_ID));        
    }
    
    /**
     * Can create a new line
     */
    public void testCreateLine() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        KPhone kphone = (KPhone) pc.newPhone(KPhone.FACTORY_ID);
        KPhoneLine line = (KPhoneLine) kphone.createLine(new LineData());
        assertNotNull(line);
    }
    
    /**
     * Basic phone and line settings are available, shows setting models are loaded into memory
     */
    public void testSettingsNotNull() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        KPhone kphone = (KPhone) pc.newPhone(KPhone.FACTORY_ID);
        assertNotNull(kphone.getSettings().getSetting("General"));
        KPhoneLine line = (KPhoneLine) kphone.createLine(new LineData());
        assertNotNull(line.getSettings().getSetting("Registration"));
    }

    /**
     * Test we get create KPhone profiles into web root directory available for download
     * @throws IOException
     */
    public void testEmptyProfile() throws Exception {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        KPhone kphone = (KPhone) pc.newPhone(KPhone.FACTORY_ID);
        kphone.getPhoneData().setSerialNumber("000000000000");
        kphone.setWebDirectory(TestHelper.getTestDirectory());
        kphone.generateProfiles();
        
        FileReader actualProfile = new FileReader(kphone.getProfileFileName());
        String actual = IOUtils.toString(actualProfile);
        actualProfile.close();
        
        InputStream expectedProfile = getClass().getResourceAsStream("empty-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        
        assertEquals(expected, actual);
    }
    
    public void testDefaultProfile() throws Exception {
        MockControl pcControl = MockControl.createNiceControl(PhoneContext.class);
        PhoneContext pc = (PhoneContext) pcControl.getMock();
        pcControl.expectAndReturn(pc.getSystemDirectory(), TestHelper.getSysDirProperties().get("sysdir.etc"), MockControl.ZERO_OR_MORE);
        pcControl.expectAndReturn(pc.newLine(KPhoneLine.FACTORY_ID), new KPhoneLine());
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

        StringWriter actualWriter = new StringWriter();
        kphone.generateProfile(actualWriter);        
        InputStream expectedProfile = getClass().getResourceAsStream("default-kphonerc");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, actualWriter.toString());
        
        pcControl.verify();
    }
}
