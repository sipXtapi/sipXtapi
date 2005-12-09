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
import java.util.List;

import junit.framework.TestCase;

import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.search.PrefixQuery;
import org.apache.lucene.search.Query;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.IdentityToBean;
import org.springframework.context.ApplicationContext;

public class SearchManagerImplTestDb extends TestCase {

    private SearchManager m_searchManager;
    private CoreContext m_coreContext;
    private IdentityToBean m_identityToBean;

    protected void setUp() throws Exception {
        ApplicationContext context = TestHelper.getApplicationContext();
        m_searchManager = (SearchManager) context.getBean(SearchManager.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) context.getBean(CoreContext.CONTEXT_BEAN_NAME);
        m_identityToBean = new IdentityToBean(m_coreContext);
        TestHelper.cleanInsert("ClearDb.xml");
        IndexManager indexManager = (IndexManager) context
                .getBean(IndexManager.CONTEXT_BEAN_NAME);
        indexManager.indexAll();
    }

    public void testSearch() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        Collection collection = m_searchManager.search("bon*", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("userName:bongo", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("firstName:bongo", m_identityToBean);
        assertEquals(0, collection.size());

        user.setUserName("kuku");
        m_coreContext.saveUser(user);

        collection = m_searchManager.search("first", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("userName:kuku", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("bongo", m_identityToBean);
        assertEquals(0, collection.size());

        user.setAliasesString("aaa, bcd");
        m_coreContext.saveUser(user);

        collection = m_searchManager.search("aaa", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search("alias:bcd", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));
    }

    public void testSearchByClass() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        Collection collection = m_searchManager.search(User.class, "bon*", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search(User.class, "userName:bongo", m_identityToBean);
        assertEquals(1, collection.size());
        assertTrue(collection.remove(user));

        collection = m_searchManager.search(Phone.class, "userName:bongo", null);
        assertEquals(0, collection.size());

        collection = m_searchManager.search(User.class, "firstName:bongo", null);
        assertEquals(0, collection.size());
    }

    public void testSearchIdent() throws Exception {
        User user = new User();
        user.setFirstName("first");
        user.setLastName("last");
        user.setUserName("bongo");

        m_coreContext.saveUser(user);

        Collection collection = m_searchManager.search("bon*", null);
        assertEquals(1, collection.size());
        Identity ident = (Identity) collection.iterator().next();
        assertEquals("last, first, bongo", ident.getName());
    }

    public void testSorting() throws Exception {
        final String[] names = {
            "aa", "bb", "ee", "zz"
        };

        for (int i = 0; i < names.length; i++) {
            User user = new User();
            user.setFirstName("first");
            user.setLastName("last");
            user.setUserName(names[i]);

            m_coreContext.saveUser(user);
        }

        // ascending
        List collection = m_searchManager.search(User.class, "first*", 0, -1, "userName", true,
                m_identityToBean);
        assertEquals(names.length, collection.size());
        User user = (User) collection.get(0);
        assertEquals(names[0], user.getUserName());
        user = (User) collection.get(names.length - 1);
        assertEquals(names[names.length - 1], user.getUserName());

        // descending
        collection = m_searchManager.search(User.class, "first*", 0, -1, "userName", false,
                m_identityToBean);
        assertEquals(names.length, collection.size());
        user = (User) collection.get(names.length - 1);
        assertEquals(names[0], user.getUserName());
        user = (User) collection.get(0);
        assertEquals(names[names.length - 1], user.getUserName());

        // do not return first item - descending order
        collection = m_searchManager.search(User.class, "first*", 1, -1, "userName", false,
                m_identityToBean);
        int size = collection.size();
        assertEquals(names.length - 1, size);
        user = (User) collection.get(0);
        assertEquals(names[names.length - 2], user.getUserName());
        user = (User) collection.get(size - 1);
        assertEquals(names[0], user.getUserName());

        // only return 2 items starting from first - ascending order
        int pageSize = 2;
        collection = m_searchManager.search(User.class, "first*", 1, pageSize, "userName", true,
                m_identityToBean);
        assertEquals(pageSize, collection.size());
        user = (User) collection.get(0);
        assertEquals(names[1], user.getUserName());
        user = (User) collection.get(1);
        assertEquals(names[2], user.getUserName());
    }

    public void testSortingIllegalFieldName() throws Exception {
        List collection = m_searchManager.search(User.class, "first*", 0, -1, "bongo", true,
                m_identityToBean);
        assertEquals(0, collection.size());
    }

    public void testParseQuery() throws Exception {
        SearchManagerImpl impl = new SearchManagerImpl();
        impl.setAnalyzer(new StandardAnalyzer());

        Query query = impl.parseUserQuery("kuku");
        assertTrue(query instanceof PrefixQuery);

        query = impl.parseUserQuery("-kuku");
        assertFalse(query instanceof PrefixQuery);

        query = impl.parseUserQuery("name:kuku");
        assertTrue(query instanceof PrefixQuery);

        query = impl.parseUserQuery("name:kuku AND bongo");
        assertFalse(query instanceof PrefixQuery);
    }
}
