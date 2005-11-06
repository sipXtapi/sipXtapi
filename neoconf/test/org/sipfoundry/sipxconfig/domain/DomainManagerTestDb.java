/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.SipxPbx;
import org.springframework.context.ApplicationContext;

public class DomainManagerTestDb extends TestCase {

    private DomainManager m_manager;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_manager = (DomainManager) app.getBean(DomainManager.BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    public void testGetDomains() throws Exception {
        TestHelper.cleanInsertFlat("domain/DomainSeed.xml");
        List domains = m_manager.getDomains();
        assertEquals(2, domains.size());
        Domain[] actual = (Domain[]) domains.toArray(new Domain[2]);
        assertTrue(actual[0] instanceof SipxPbx);        
        assertFalse(actual[1] instanceof SipxPbx);        
    }
}
