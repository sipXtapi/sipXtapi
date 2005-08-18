/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;
import java.util.Iterator;
import java.util.Map;

import org.hibernate.EmptyInterceptor;
import org.hibernate.type.Type;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public class DaoEventDispatcher extends EmptyInterceptor implements BeanFactoryAware {

    private ListableBeanFactory m_beanFactory;
    
    /**
     * This can only be used withy listeable bean factory
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
    
    public BeanFactory getBeanFactory() {
        return m_beanFactory;
    }

    public void onDelete(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {

        Map beanMap = m_beanFactory.getBeansOfType(DaoEventListener.class, true, true);
        Iterator beans = beanMap.values().iterator();
        while (beans.hasNext()) {
            DaoEventListener listener = (DaoEventListener) beans.next();
            listener.onDelete(entity, id, state, propertyNames, types);
        }
    }

    public boolean onSave(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        
        boolean modified = false;
        Map beanMap = m_beanFactory.getBeansOfType(DaoEventListener.class, true, true);
        Iterator beans = beanMap.values().iterator();
        while (beans.hasNext()) {
            DaoEventListener listener = (DaoEventListener) beans.next();
            if (listener.onSave(entity, id, state, propertyNames, types)) {
                modified = true;
            }            
        }

        return modified;
    }    
}
