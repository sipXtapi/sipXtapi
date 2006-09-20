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
package org.sipfoundry.sipxconfig.common.event;

public abstract class EntityDeleteListener<T> implements DaoEventListener {
    private Class<T> m_klass;

    public EntityDeleteListener(Class<T> klass) {
        m_klass = klass;
    }

    public void onDelete(Object entity) {
        if (m_klass.isAssignableFrom(entity.getClass())) {
            onEntityDelete((T) entity);
        }
    }

    public void onSave(Object entity_) {
    }

    protected abstract void onEntityDelete(T group);
}
