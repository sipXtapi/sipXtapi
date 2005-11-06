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

import java.io.Serializable;

import org.apache.lucene.document.Document;
import org.apache.lucene.index.Term;
import org.hibernate.type.Type;

public interface BeanAdaptor {

    /**
     * @return true if the document should be added to index
     */
    public abstract boolean documentFromBean(Document document, Object bean, Serializable id,
            Object[] state, String[] fieldNames, Type[] types);

    public abstract Term getIdentityTerm(Object bean, Serializable id);

    public abstract Identity getBeanIdentity(Document document);

    public static class Identity {
        private Class m_klass;
        private Serializable m_id;

        public Identity(Class klass, Serializable id) {
            super();
            // TODO Auto-generated constructor stub
            m_klass = klass;
            m_id = id;
        }

        public Serializable getBeanId() {
            return m_id;
        }

        public Class getBeanClass() {
            return m_klass;
        }
    }
}
