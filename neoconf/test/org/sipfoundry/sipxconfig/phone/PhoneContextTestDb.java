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

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.vendor.Polycom;
import org.sipfoundry.sipxconfig.vendor.PolycomPhone;

/**
 * Requires Database access.
 */
public class PhoneContextTestDb extends TestCase {

    private PhoneContext m_context;

    protected void setUp() throws Exception {
        super.setUp();
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
    }

    protected void tearDown() throws Exception {
        TestHelper.tearDown();
    }

    public void testSampleData() {
        assertNotNull(createSampleCredential());
        assertNotNull(createSampleEndpoint());
    }
    
    public void testSampleEndpointSettings() {
        Endpoint endpoint = createSampleEndpoint();
        Phone phone = m_context.getPhone(endpoint);
        SettingGroup group = phone.getSettingGroup();
        Setting displayName = group.getSetting(PolycomPhone.REGISTRATION_SETTINGS).getSetting("displayName");
        displayName.setValue("blah blah");
        m_context.storeEndpoint(endpoint);
        TestHelper.deleteOnTearDown(endpoint);
    }

    public void testLoadPhoneSummaries() {
        int preSize = m_context.loadPhoneSummaries().size();

        assertNotNull(createSampleEndpoint());
        
        // just test there's one more in list, not a very 
        // hard test
        List summaries = m_context.loadPhoneSummaries();
        assertEquals(preSize + 1, summaries.size());
    }
    
    public void testLoadUserByTemplateUsersSingleExpression() {
        User user = createSampleUser();
        User template = new User();
        // pick unique field
        template.setExtension(user.getExtension());       
        List users = m_context.loadUserByTemplateUser(template);
        assertEquals(1, users.size());
        User results = (User) users.get(0);
        assertEquals(user.getId(), results.getId());
    }
    
    public void testLoadUserByTemplateUsersMultiExpression() {
        User template = createSampleUser();
        List users = m_context.loadUserByTemplateUser(template);
        assertTrue(users.size() >= 1);
    }
    

    /**
     * PhoneSummaryFactory implementation
     */
    public PhoneSummary createPhoneSummary() {
        return new PhoneSummary();
    }

    public User createSampleUser() {
        User user = new User();
        user.setExtension("0000000000"); // assumption, unique
        user.setPassword("any-password");
        user.setFirstName("Test");
        user.setLastName("User");
        user.setDisplayId("phonecontext-test-user");
        user.setOrganization(m_context.loadRootOrganization());
        m_context.saveUser(user);
        TestHelper.deleteOnTearDown(user);
        
        return user;
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */
    public Endpoint createSampleEndpoint() {        
        Endpoint endpoint = new Endpoint();
        // assumption that this is unique
        endpoint.setSerialNumber("f34298760024fcc1"); 
        // would use generic, but this has setting values 
        endpoint.setPhoneId(Polycom.MODEL_300.getModelId());
        m_context.storeEndpoint(endpoint);
        TestHelper.deleteOnTearDown(endpoint);
        
        return endpoint;
    }
    
    /**
     * Create some generic sample data, destroyed verifyDataUnaltered
     */    
    public Credential createSampleCredential() {
        Credential cred = new Credential();
        m_context.storeCredential(cred);
        TestHelper.deleteOnTearDown(cred);
        
        return cred;
    }
        
    /** 
     * Most data can be cleared by running patch, run this test as application to clear
     * test data that exists in other tables from misbehaved unittests
     */
    public static void main(String[] args) {
        // total reset of data
        PhoneContext c = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        User testUser = c.loadUserByDisplayId("phonecontext-test-user");
        if (testUser != null) {
            c.deleteUser(testUser);
        }
    }
}