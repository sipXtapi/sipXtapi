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

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class DaoUtilsTestDb extends SipxDatabaseTestCase {
    private SipxHibernateDaoSupport m_dao;
    private HibernateTemplate m_hibernate;

    protected void setUp() throws Exception {
        m_dao = SipxHibernateDaoSupportTestDb.createDao();
        m_hibernate = m_dao.getHibernateTemplate();
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testCheckDuplicates() throws Exception {
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        User user = new User();
        user.setUserName("userseed1");
        assertTrue(DaoUtils.checkDuplicates(m_hibernate, User.class, user, "userName", null));

        user = new User();
        user.setUserName("wont find this guy");
        assertFalse(DaoUtils.checkDuplicates(m_hibernate, User.class, user, "userName", null));
    }
}
