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

import junit.framework.TestCase;

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Requires Database access.
 */
public class PhoneDaoImplTestDb extends TestCase implements PhoneSummaryFactory {

    private PhoneDao m_dao;

    private PhoneContext m_context;

    private UnitTestDao m_testData;

    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();

        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest-db");

        m_context = (PhoneContext) bf.getFactory().getBean("phoneContext");
        assertNotNull(m_context);

        // now use some bean from factory 
        m_dao = (PhoneDao) bf.getFactory().getBean("phoneDao");
        assertNotNull(m_dao);

        m_testData = (UnitTestDao) bf.getFactory().getBean("testData");
        assertNotNull(m_testData);
        assertTrue(m_testData.initializeImmutableData());
    }

    protected void tearDown() throws Exception {
        assertTrue(m_testData.verifyDataUnaltered());
    }

    /**
     * This really tests UnitTestDao object
     */
    public void testSampleData() {
        User user = m_dao.loadUser(m_testData.getTestUserId());
        assertNotNull(m_testData.createSampleSettingSet());
        Endpoint endpoint = m_testData.createSampleEndpoint();
        assertNotNull(m_testData.createSampleEndpointAssignment(endpoint, user));
        //m_dao.flush();
    }

    public void testLoadPhoneSummaries() {
        int preSize = m_dao.loadPhoneSummaries(this).size();

        Endpoint endpoint = m_testData.createSampleEndpoint();
        User user = m_dao.loadUser(m_testData.getTestUserId());
        EndpointAssignment assignment = m_testData.createSampleEndpointAssignment(endpoint, user);
        assertNotNull(assignment);

        // just test there's one more in list, not a very 
        // hard test
        assertEquals(preSize + 1, m_dao.loadPhoneSummaries(this).size());
    }

    /**
     * PhoneSummaryFactory implementation
     */
    public PhoneSummary createPhoneSummary() {
        return new PhoneSummary();
    }

    /**
     * PhoneSummaryFactory implementation
     */
    public PhoneContext getPhoneContext() {
        return m_context;
    }

    public void testLoadUserByDisplayId() {
        User testUser = m_dao.loadUser(m_testData.getTestUserId());
        User byDisplayId = m_dao.loadUserByDisplayId(testUser.getDisplayId());
        assertNotNull(byDisplayId);
    }
}