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
package org.sipfoundry.sipxconfig.search;

import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.TestHelper.TestCaseDb;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class IndexManagerImplTestDb extends TestCaseDb {

    private SearchManager m_searchManager;
    private IndexManager m_indexManager;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_searchManager = (SearchManager) context.getBean(SearchManager.CONTEXT_BEAN_NAME);
        m_indexManager = (IndexManager) context.getBean(IndexManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("common/UserSearchSeed.xml");

        m_indexManager.indexAll();
    }

    public void testSearchByName() throws Exception {
        List users = m_searchManager.search("u*2");
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertNotNull(users.iterator().next());
        assertEquals("userseed2", user.getUserName());
    }

    public void testSearchByAlias() throws Exception {
        List users = m_searchManager.search("two");
        assertEquals(1, users.size());
        User user = (User) users.get(0);
        assertEquals("userseed2", user.getUserName());
    }

}
