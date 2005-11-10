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
    public void testGetBeanIndentity() throws Exception {
        DefaultBeanAdaptor adaptor = new DefaultBeanAdaptor();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:36"));

        Identity beanIdentity = adaptor.getBeanIdentity(doc);
        assertSame(User.class, beanIdentity.getBeanClass());
        assertEquals(new Integer(36), beanIdentity.getBeanId());
    }

    public void testGetBeanIndentityWrongClass() throws Exception {
        DefaultBeanAdaptor adaptor = new DefaultBeanAdaptor();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.Xyz:36"));

        Identity beanIdentity = adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetBeanIndentityWrongId() throws Exception {
        DefaultBeanAdaptor adaptor = new DefaultBeanAdaptor();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:aaa"));

        Identity beanIdentity = adaptor.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetIndentityTerm() throws Exception {
        DefaultBeanAdaptor adaptor = new DefaultBeanAdaptor();

        User user = new User();
        user.setUniqueId();

        Term identityTerm = adaptor.getIdentityTerm(user, user.getId());

        Term expectedTerm = new Term("id", "org.sipfoundry.sipxconfig.common.User:"
                + user.getId());
        assertEquals(expectedTerm, identityTerm);
    }

    public void testIndexClass() {
        DefaultBeanAdaptor adaptor = new DefaultBeanAdaptor();

        Document doc = new Document();
        adaptor.indexClass(doc, User.class);

        assertEquals(User.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        adaptor.indexClass(doc, SnomPhone.class);
        // snom phone should be indexed as phone
        assertEquals(Phone.class.getName(), doc.get(Indexer.CLASS_FIELD));

        doc = new Document();
        adaptor.indexClass(doc, String.class);

        assertNull(doc.get(Indexer.CLASS_FIELD));
    }
}
