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
import org.sipfoundry.sipxconfig.phone.Credential;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.phone.PhoneTestHelper;
import org.sipfoundry.sipxconfig.phone.SettingSet;
import org.sipfoundry.sipxconfig.phone.User;

public class PolycomPhoneTest extends XMLTestCase {
    
    private MockControl m_control;
    
    private PhoneContext m_phoneContext;
    
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);

        PhoneTestHelper helper = PhoneTestHelper.createHelper();
        m_phoneContext = helper.getPhoneContext();
        
        m_control = MockControl.createStrictControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) m_control.getMock();
        Organization rootOrganization = new Organization();
        rootOrganization.setDnsDomain("hostname.domainname");
        m_control.expectAndReturn(dao.loadRootOrganization(), rootOrganization);        

        m_phoneContext.setPhoneDao(dao);
    }
    
    public void tearDown() {
    }
    
    public void testFactoryCreation() {
        m_control.replay();

        Endpoint endpoint = new Endpoint();
        endpoint.setSettings(new SettingSet());
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        Phone phone = m_phoneContext.getPhone(endpoint);
        
        assertNotNull(phone);

        m_control.verify();
    }
    
    public void testBasicProfile() throws Exception {
        m_control.replay();        
        
        // create basic data
        Endpoint endpoint = new Endpoint();
        endpoint.setSettings(new SettingSet());
        endpoint.setSerialNumber("123abc");
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        PolycomPhone phone = (PolycomPhone) m_phoneContext.getPhone(endpoint);

        Line line = new Line();
        User user = new User();
        Credential credential = new Credential();
        user.setCredential(credential);
        line.setEndpoint(endpoint);
        line.setUser(user);
        
        phone.generateProfiles();
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
        
        m_control.verify();
    }       
}
