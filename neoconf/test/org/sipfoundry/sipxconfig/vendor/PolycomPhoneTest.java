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
package org.sipfoundry.sipxconfig.vendor;

import java.io.FileReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Iterator;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

public class PolycomPhoneTest extends XMLTestCase {
        
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    public void testNop() {
        // remove this
    }
    
    public void _testBasicProfile() throws Exception {
        MockControl phoneControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneControl.getMock();
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");
        phoneControl.replay();

        PolycomPhone phone = new PolycomPhone();
        phone.setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        phone.setModelId(Polycom.MODEL_600.getModelId());

        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber("0004f200e06b");
        endpoint.setPhoneId(phone.getModelId());
        
        User user = new User();
        user.setDisplayId("joe");
        user.setFirstName("Joe");
        user.setLastName("Taylor");
        
        Line line = new Line();
        line.setUser(user);
        endpoint.addLine(line);
        
        PolycomPhoneConfig config = new PolycomPhoneConfig();
        config.setTftpRoot(TestHelper.getTestDirectory());
        config.setVelocityEngine(TestHelper.getVelocityEngine());
        phone.setConfig(config);
        
        ApplicationConfiguration app = new ApplicationConfiguration(phone, endpoint);
        
        
        phone.generateProfiles(phoneContext, endpoint);
        InputStream expectedPhoneStream = null;
        InputStream actualPhoneStream = null;
        try {            
            expectedPhoneStream = getClass().getResourceAsStream("basicProfile/0004f200e06b-phone.cfg");
            assertNotNull(expectedPhoneStream);
            Reader expectedXml = new InputStreamReader(expectedPhoneStream);
            
            Reader generatedXml = new FileReader(app.getAppFilename());

            Diff phoneDiff = new Diff(expectedXml, generatedXml);
            assertXMLEqual(phoneDiff, true);
        } finally {
            if (expectedPhoneStream != null) {
                expectedPhoneStream.close();
            }
            if (actualPhoneStream != null) {
                actualPhoneStream.close();
            }
        }
        
        phoneControl.verify();
    }
    
    public void _testSettingModel() {
        PolycomPhone phone = new PolycomPhone();
        String sysDir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        assertNotNull(sysDir);
        phone.setSystemDirectory(sysDir);
        SettingGroup model = phone.getSettingModel(new Endpoint());
        assertNotNull(model);
        
        SettingGroup lines = (SettingGroup) model.getSetting(Phone.LINE_SETTINGS);
        assertNotNull(lines);
        
        Iterator headings = model.getValues().iterator();
        assertEquals("Registration", nextModel(headings).getLabel());
        assertEquals("Lines", nextModel(headings).getLabel());
        assertFalse(headings.hasNext());
    }
    
    private SettingGroup nextModel(Iterator i) {
        return (SettingGroup) i.next();
    }
}


