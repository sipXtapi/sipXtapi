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

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Requires Database access.
 */
public class PhoneDaoImplTestDb extends TestCase {

    private PhoneDao m_dao;
    
    private Endpoint m_teardownEndpoint;
    
    private EndpointAssignment m_teardownAssignment;
    
    private SettingSet m_teardownSettings;

    private UnitTestDao m_testData;       
    
    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();
        
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest-db");
        // now use some bean from factory 
        m_dao = (PhoneDao) bf.getFactory().getBean("phoneDao");
        assertNotNull(m_dao);
        
        m_testData = (UnitTestDao) bf.getFactory().getBean("testData");
        assertNotNull(m_testData);
        assertTrue(m_testData.initializeImmutableData());
    }
    
    protected void tearDown() throws Exception {
        // NOTE: If unittest fails to tear down, next unittest run
        // may fail due to duplicate data errors
        if (m_teardownSettings != null) {
            m_dao.deleteSetting(m_teardownSettings);
            m_teardownSettings = null;
        }
        if (m_teardownAssignment != null) {
            m_dao.deleteEndpointAssignment(m_teardownAssignment);
            m_teardownAssignment = null;
        }
        if (m_teardownEndpoint != null) {
            m_dao.deleteEndpoint(m_teardownEndpoint);
            m_teardownEndpoint = null;
        }
        m_dao.flush();
        assertTrue(m_testData.verifyDataUnaltered());
    }
    
    /**
     * Tests a number of sattelite objects to endpoint as well
     */
    public void testEndpoint() {  
        
        Organization org = m_dao.loadRootOrganization();
        assertEquals(1, org.getId());
                       
        User user = m_dao.loadUser(m_testData.getTestUserId());

        SettingSet root = new SettingSet("root");       
        SettingSet subset = new SettingSet("subset");
        root.addSetting(subset);
        Setting setting = new Setting("subsetting", "value");
        subset.addSetting(setting);
        
        m_dao.storeSetting(root, PhoneDao.CASCADE);
        m_teardownSettings = root;
        
        SettingSet rootRead = m_dao.loadSettings(root.getId());
        assertNotNull(rootRead);
        assertEquals(1, rootRead.getSettings().size());

        Endpoint endpoint = new Endpoint();
        // assumption that this is unique
        endpoint.setSerialNumber("f34298760024fcc1"); 
        endpoint.setPhoneId("unittest - PhoneDaoImplTestDb");
        m_dao.storeEndpoint(endpoint);
        m_teardownEndpoint = endpoint;
        
        EndpointAssignment assignment = new EndpointAssignment();
        assignment.setUser(user);
        assignment.setEndpoint(endpoint);
        assignment.setLabel("work phone");
        m_dao.storeEndpointAssignment(assignment);                
        m_teardownAssignment = assignment;

        m_dao.flush();
    }
    
    public void testLoadEndpoints() {

        Endpoint endpoint = new Endpoint();
        // assumption that this is unique
        endpoint.setSerialNumber("f34298760024fcc2"); // assumption: unique 
        endpoint.setPhoneId("unittest - PhoneDaoImplTestDb");
        m_dao.storeEndpoint(endpoint);
        
        m_teardownEndpoint = endpoint;
        
        m_dao.flush();
        
        List endpoints = m_dao.loadEndpoints();
        assertTrue(endpoints.size() > 0);        
    }
}
