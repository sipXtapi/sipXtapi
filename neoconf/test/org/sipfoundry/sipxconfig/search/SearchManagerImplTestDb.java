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

import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class SearchManagerImplTestDb extends TestCase {

    private SearchManager m_searchManager;
    private CoreContext m_coreContext;

    protected void setUp() {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_searchManager = (SearchManager) context.getBean(SearchManager.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) context.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_coreContext.clear();
    }

    public void testSearch() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        Collection collection = m_searchManager.search("bon*");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("userName:bongo");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("firstName:bongo");
        assertEquals(0, collection.size());

        user.setUserName("kuku");
        m_coreContext.saveUser(user);

        collection = m_searchManager.search("first");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("userName:kuku");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("bongo");
        assertEquals(0, collection.size());

        user.setAliasesString("aaa, bcd");
        m_coreContext.saveUser(user);

        collection = m_searchManager.search("aaa");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("alias:bcd");
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));
    }
}
