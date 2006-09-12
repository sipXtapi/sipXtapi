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
        TestHelper.cleanInsertFlat("domain/DomainSeed.xml");
        Domain d = m_context.getDomain();
        assertNotNull(d);        
    }

    public void testUpdateDomain() throws Exception {
        Domain d = new Domain();
        d.setName("robin");
        m_context.saveDomain(d);
        ReplacementDataSet ds = TestHelper.loadReplaceableDataSetFlat("domain/DomainUpdateExpected.xml");
        ds.addReplacementObject("[domain_id]", d.getId());
        ITable actual = ds.getTable("domain");
        ITable expected = TestHelper.getConnection().createDataSet().getTable("domain");
        Assertion.assertEquals(expected, actual);
    }
}
