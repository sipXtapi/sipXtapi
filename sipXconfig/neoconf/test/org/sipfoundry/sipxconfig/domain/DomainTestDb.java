/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.DomainManager.DomainNotInitializedException;
import org.springframework.context.ApplicationContext;

public class DomainTestDb extends TestCase {
    
    private DomainManager m_context;

    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (DomainManager) m_appContext.getBean(DomainManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetDomain() throws Exception {
        Domain d = m_context.getDomain();
        assertNotNull(d);        
    }
    
    public void testGetEmptyDomain() throws Exception {
        TestHelper.cleanInsert("domain/NoDomainSeed.xml");        
        try {
            m_context.getDomain();
            fail();
        } catch (DomainNotInitializedException expected) {
            assertTrue(true);
        }
    }    

    public void testSaveNewDomain() throws Exception {
        Domain d = new Domain();
        d.setName("robin");
        m_context.saveDomain(d);
        assertDomainEquals(d, "domain/DomainUpdateExpected.xml");
    }

    public void testUpdateDomain() throws Exception {
        Domain domain = m_context.getDomain();
        domain.setName("robin");
        m_context.saveDomain(domain);        
        assertDomainEquals(domain, "domain/DomainUpdateExpected.xml");
    }
    
    public void testSaveAliases() throws Exception {
        Domain domain = m_context.getDomain();
        domain.addAlias("waxwing");
        m_context.saveDomain(domain);

        ReplacementDataSet ds = TestHelper.loadReplaceableDataSetFlat("domain/ExpectedDomainAliases.xml");
        ds.addReplacementObject("[domain_id]", domain.getId());
        ITable expected = ds.getTable("domain_alias");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("domain_alias");
        Assertion.assertEquals(expected, actual);                      
    }
    
    private void assertDomainEquals(Domain domain, String expectedFile) throws Exception {
        ReplacementDataSet ds = TestHelper.loadReplaceableDataSetFlat(expectedFile);
        ds.addReplacementObject("[domain_id]", domain.getId());
        ITable expected = ds.getTable("domain");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);        
    }
}
