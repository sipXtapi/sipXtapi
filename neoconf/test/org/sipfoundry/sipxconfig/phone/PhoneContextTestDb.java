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

/**
 * Requires Database access.
 */
public class PhoneContextTestDb extends TestCase {

    private PhoneContext m_context;

    private UnitTestDao m_testData;

    protected void setUp() throws Exception {
        super.setUp();
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(m_context);

        m_testData = PhoneTestHelper.getUnitTestDao();
        assertNotNull(m_testData);
        assertTrue(m_testData.initializeImmutableData());        
    }

    protected void tearDown() throws Exception {
        assertTrue(m_testData.verifyDataUnaltered());
        m_context.flush();
    }

    public void testSampleData() {
        assertNotNull(m_testData.createSampleCredential());
        assertNotNull(m_testData.createSampleEndpoint());
    }
    
    public void testSampleEndpointLine() {
        User user = m_context.loadUser(m_testData.getTestUserId());
        assertNotNull(m_testData.createSampleLine(user));        
    }

    public void testLoadPhoneSummaries() {
        int preSize = m_context.loadPhoneSummaries().size();

        User user = m_context.loadUser(m_testData.getTestUserId());
        Line line = m_testData.createSampleLine(user);
        assertNotNull(line);
        
        // just test there's one more in list, not a very 
        // hard test
        List summaries = m_context.loadPhoneSummaries();
        assertEquals(preSize + 1, summaries.size());
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
}