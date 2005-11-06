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
import org.sipfoundry.sipxconfig.search.BeanIndexer.Identity;

public class DefaultBeanIndexerTest extends TestCase {
    public void testGetBeanIndentity() throws Exception {
        DefaultBeanIndexer indexer = new DefaultBeanIndexer();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:36"));

        Identity beanIdentity = indexer.getBeanIdentity(doc);
        assertSame(User.class, beanIdentity.getBeanClass());
        assertEquals(new Integer(36), beanIdentity.getBeanId());
    }

    public void testGetBeanIndentityWrongClass() throws Exception {
        DefaultBeanIndexer indexer = new DefaultBeanIndexer();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.Xyz:36"));

        Identity beanIdentity = indexer.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetBeanIndentityWrongId() throws Exception {
        DefaultBeanIndexer indexer = new DefaultBeanIndexer();

        // cannot mock documents
        Document doc = new Document();
        doc.add(Field.Keyword("id", "org.sipfoundry.sipxconfig.common.User:aaa"));

        Identity beanIdentity = indexer.getBeanIdentity(doc);
        assertNull(beanIdentity);
    }

    public void testGetIndentityTerm() throws Exception {
        DefaultBeanIndexer indexer = new DefaultBeanIndexer();

        User user = new User();
        user.setUniqueId();

        Term identityTerm = indexer.getIdentityTerm(user, user.getId());

        Term expectedTerm = new Term("id", "org.sipfoundry.sipxconfig.common.User:"
                + user.getId());
        assertEquals(expectedTerm, identityTerm);
    }
}
