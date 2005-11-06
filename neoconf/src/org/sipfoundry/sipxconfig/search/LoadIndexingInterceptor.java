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

import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.common.SpringHibernateInstantiator;

/**
 * This is used to indexing on load
 */
public class LoadIndexingInterceptor extends SpringHibernateInstantiator {
    private Indexer m_indexer;

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    public boolean onLoad(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        m_indexer.indexBean(entity, id, state, propertyNames, types);
        return false;
    }
}
