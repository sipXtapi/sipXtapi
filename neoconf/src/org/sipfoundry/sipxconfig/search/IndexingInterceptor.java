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

import org.hibernate.EmptyInterceptor;
import org.hibernate.type.Type;

public class IndexingInterceptor extends EmptyInterceptor {
    private IndexManager m_indexManager;

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public boolean onSave(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        m_indexManager.indexBean(entity, id, state, propertyNames, types);
        return false;
    }

    public void onDelete(Object entity, Serializable id, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        m_indexManager.removeBean(entity, id);
    }

    public boolean onFlushDirty(Object entity, Serializable id, Object[] currentState,
            Object[] previousState_, String[] propertyNames, Type[] types) {
        m_indexManager.indexBean(entity, id, currentState, propertyNames, types);
        return false;
    }
}
