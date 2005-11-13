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

import junit.framework.TestCase;

import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.index.Term;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.snom.SnomPhone;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;

public class DefaultBeanAdaptorTest extends TestCase {
    private DefaultBeanAdaptor m_adaptor;

    protected void setUp() throws Exception {
        m_adaptor = new DefaultBeanAdaptor();
    }

    public void testGetBeanIndentity() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:36"));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertSame(User.class, beanIdentity.getBeanClass());
        assertEquals(new Integer(36), beanIdentity.getBeanId());
    }

    public void testGetBeanIndentityWrongClass() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.Xyz:36"));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetBeanIndentityWrongId() throws Exception {
        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:aaa"));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetIndentityTerm() throws Exception {
        User user = new User();
        user.setUniqueId();

        Term identityTerm = m_adaptor.getIdentityTerm(user, user.getId());

        Term expectedTerm = new Term("id", "org.sipfoundry.sipxconfig.common.User:"
                + user.getId());
        assertEquals(expectedTerm, identityTerm);
    }

    public void testIndexClass() {
        Document doc = new Document();
        m_adaptor.indexClass(doc, User.class);

        assertEquals(User.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        m_adaptor.indexClass(doc, SnomPhone.class);
        // snom phone should be indexed as phone
        assertEquals(Phone.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        m_adaptor.indexClass(doc, String.class);

        assertNull(doc.get(Indexer.CLASS_FIELD));
    }

    public void test() {
        Document doc = new Document();
        doc.add(Field.Text("name", "abc"));
        doc.add(Field.Text("extension", "1234"));
        doc.add(Field.Text("description", "bongo"));

        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:36"));

        Identity beanIdentity = m_adaptor.getBeanIdentity(doc);
        assertEquals("bongo", beanIdentity.getDescription());
        assertEquals("abc, 1234", beanIdentity.getName());
    }
}
