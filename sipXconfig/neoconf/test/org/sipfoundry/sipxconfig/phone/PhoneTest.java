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
import java.io.OutputStream;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;

public class PhoneTest extends TestCase {

    public void testSetSerialNumber() {
        assertEquals("123456789012", Phone.cleanSerialNumber("123456789012"));
        assertEquals("123456789012", Phone.cleanSerialNumber("1234 5678 9012"));
        assertEquals("123456789012", Phone.cleanSerialNumber("12:34:56:78:90:12"));
        assertEquals("aabbccddeeff", Phone.cleanSerialNumber("AABBCCDDEEFF"));
        assertEquals("totallybogus", Phone.cleanSerialNumber("totallybogus"));
        assertNull(Phone.cleanSerialNumber(null));
    }

    public void testGenerateAndRemoveProfiles() {
        final AbstractProfileGenerator generator = new AbstractProfileGenerator() {
            protected void generateProfile(ProfileContext context, String templateFileName,
                     OutputStream out) throws IOException {
                IOUtils.write("profile", out);
            }
        };
        
        String root = TestHelper.getTestDirectory() + "/phone";
        FileSystemProfileLocation location = new FileSystemProfileLocation();
        location.setParentDir(root);
        
        generator.setProfileLocation(location);

        Phone phone = new TestPhone() {
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

        phone.setProfileGenerator(generator);

        String phoneFilename = phone.getPhoneFilename();
        System.err.println(phoneFilename);
        File profile = new File(phoneFilename);
        phone.generateProfiles();
        assertTrue(profile.exists());
        phone.removeProfiles();
        assertFalse(profile.exists());
    }

    public void testFindByUsername() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("sipfoundry.org"));
        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContextCtrl.replay();

        Phone phone = new AcmePhone();
        phone.setPhoneContext(phoneContext);
        phone.setSerialNumber("123456789012");
        Line l = phone.createLine();
        User u = new User();
        u.setUserName("foo");
        l.setUser(u);
        phone.addLine(l);

        assertSame(l, phone.findByUsername("foo"));
        assertNull(phone.findByUsername("foox"));
        assertNull(phone.findByUsername("Foo"));

        phoneContextCtrl.verify();
    }

    public void testFindByUri() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setDomainManager(TestHelper.getTestDomainManager("sipfoundry.org"));
        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.getPhoneDefaults();
        phoneContextCtrl.andReturn(defaults).atLeastOnce();
        phoneContextCtrl.replay();

        Phone phone = new AcmePhone();
        phone.setPhoneContext(phoneContext);
        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        phone.setSerialNumber("123456789012");
        Line l = phone.createLine();
        phone.addLine(l);
        LineInfo info = new LineInfo();
        info.setUserId("foo");
        info.setRegistrationServer("sipfoundry.org");
        phone.setLineInfo(l, info);
        l.setModelFilesContext(TestHelper.getModelFilesContext());

        assertSame(l, phone.findByUri("sip:foo@sipfoundry.org"));
        assertNull(phone.findByUri("sip:foox@sipfoundry.org"));
        assertNull(phone.findByUri("foo@sipfoundry.org"));

        phoneContextCtrl.verify();
    }

    public void testMaxLines() {
        Phone phone = new LimitedPhone();
        phone.addLine(phone.createLine());
        try {
            phone.addLine(phone.createLine());
            fail();
        } catch (Phone.MaxLinesException expected) {
            assertTrue(true);
        }
    }

    static class LimitedPhone extends Phone {
        LimitedPhone() {
            super("beanId");
            setModel(new LimitedLinePhoneModel());
        }

        @Override
        public void setLineInfo(Line line, LineInfo externalLine) {
        }

        @Override
        public LineInfo getLineInfo(Line line) {
            return null;
        }

        @Override
        public void initializeLine(Line l) {
        }
    }

    static class LimitedLinePhoneModel extends PhoneModel {
        LimitedLinePhoneModel() {
            setMaxLineCount(1);
        }
    }
}
