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
package org.sipfoundry.sipxconfig.common;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.springframework.context.ApplicationContext;
import org.springframework.orm.hibernate3.HibernateTemplate;

import junit.framework.TestCase;

public class DaoUtilsTestDb extends TestCase {
    private SipxHibernateDaoSupport m_dao;
    private HibernateTemplate m_hibernate;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_dao = (SipxHibernateDaoSupport) app.getBean(SipxHibernateDaoSupport.CONTEXT_BEAN_NAME);
        m_hibernate = m_dao.getHibernateTemplate();
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testCheckDuplicates() throws Exception {
        TestHelper.cleanInsertFlat("gateway/SeedGateway.xml");

        Gateway gateway = new Gateway();
        gateway.setName("test gateway");
        assertTrue(DaoUtils.checkDuplicates(m_hibernate, gateway, "name", null));

        gateway = new Gateway();
        gateway.setName("won't find this guy");
        assertFalse(DaoUtils.checkDuplicates(m_hibernate, gateway, "name", null));
    }

}
