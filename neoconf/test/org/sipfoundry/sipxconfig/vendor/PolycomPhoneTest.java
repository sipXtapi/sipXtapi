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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Credential;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.SettingSet;
import org.sipfoundry.sipxconfig.phone.User;

public class PolycomPhoneTest extends XMLTestCase {
        
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    public void testBasicProfile() throws Exception {
        MockControl phoneControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneControl.getMock();
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");
        phoneControl.expectAndReturn(phoneContext.loadRootOrganization(), rootOrg);
        phoneControl.replay();

        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber("123abc");
        endpoint.setSettings(new SettingSet());
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        PolycomPhone phone = new PolycomPhone();
        phone.setTftpRoot(TestHelper.getTestDirectory());
        phone.setModelId(PolycomPhone.MODEL_300);
        phone.setEndpoint(endpoint);
        phone.setVelocityEngine(TestHelper.getVelocityEngine());

        // create basic data
        Line line = new Line();
        User user = new User();
        Credential credential = new Credential();
        user.setCredential(credential);
        line.setEndpoint(endpoint);
        line.setUser(user);
        
        phone.generateProfiles(phoneContext);
        InputStream expectedPhoneStream = null;
        InputStream actualPhoneStream = null;
        try {            
            expectedPhoneStream = getClass().getResourceAsStream("phone1.cfg");
            assertNotNull(expectedPhoneStream);
            Reader expectedXml = new InputStreamReader(expectedPhoneStream);
            
            actualPhoneStream = phone.getPhoneConfigFile();
            assertNotNull(actualPhoneStream);
            Reader generatedXml = new InputStreamReader(actualPhoneStream);

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
}
