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
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.io.IOException;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;

public class PhoneTest extends TestCase {

    public void testSetSerialNumber() {
        assertEquals("123456789012", Phone.cleanSerialNumber("123456789012"));
        assertEquals("123456789012", Phone.cleanSerialNumber("1234 5678 9012"));
        assertEquals("123456789012", Phone.cleanSerialNumber("12:34:56:78:90:12"));
        assertEquals("aabbccddeeff", Phone.cleanSerialNumber("AABBCCDDEEFF"));
        assertEquals("totallybogus", Phone.cleanSerialNumber("totallybogus"));
    }

    public void testGenerateAndRemoveProfiles() {
        Phone phone = new Phone() {
            private String m_name;

            public String getPhoneFilename() {
                if (m_name != null) {
                    return m_name;
                }
                try {
                    File f = File.createTempFile("phone", "cfg");
                    f.delete();
                    m_name = f.getPath();
                    return m_name;
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        };

        String phoneFilename = phone.getPhoneFilename();
        System.err.println(phoneFilename);
        File profile = new File(phoneFilename);
        phone.generateProfiles();
        assertTrue(profile.exists());
        phone.removeProfiles();
        assertFalse(profile.exists());
    }
    
    public void testFindByUsername() {
        Phone phone = new Phone();
        phone.setSerialNumber("123456789012");
        Line l = phone.createLine();
        User u = new User();
        u.setUserName("foo");
        l.setUser(u);
        phone.addLine(l);
        
        assertSame(l, phone.findByUsername("foo"));
        assertNull(phone.findByUsername("foox"));        
        assertNull(phone.findByUsername("Foo"));        
    }
    
    public void testFindByUri() {
        PhoneDefaults defaults = new PhoneDefaults();
        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();
        phoneContextCtrl.expectAndReturn(phoneContext.getPhoneDefaults(), defaults, MockControl.ONE_OR_MORE);
        phoneContextCtrl.replay();
        
        Phone phone = new Phone();
        phone.setPhoneContext(phoneContext);
        phone.setSettingModel(TestHelper.loadSettings("unmanagedPhone/phone.xml"));
        phone.setSerialNumber("123456789012");
        Line l = phone.createLine();        
        phone.addLine(l);
        l.setSettingModel(TestHelper.loadSettings("unmanagedPhone/line.xml"));
        LineSettings settings = (LineSettings) l.getAdapter(LineSettings.class);
        settings.setRegistrationServer("sipfoundry.org");
        settings.setUserId("foo");
        
        assertSame(l, phone.findByUri("sip:foo@sipfoundry.org"));
        assertNull(phone.findByUri("sip:foox@sipfoundry.org"));        
        assertNull(phone.findByUri("foo@sipfoundry.org"));        

        phoneContextCtrl.verify();
    }

    public void testMaxLines() {
        Phone phone = new Phone(new LimitedLinePhoneModel());
        phone.addLine(phone.createLine());
        try {
            phone.addLine(phone.createLine());
            fail();
        } catch (Phone.MaxLinesException expected) {
            assertTrue(true);
        }
    }
    
    static class LimitedLinePhoneModel extends PhoneModel {
        LimitedLinePhoneModel() {
            super("beanId", "modelId", "LimitedLinePhoneModel", 1);
        }
    }
}
