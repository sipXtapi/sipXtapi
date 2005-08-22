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

import java.lang.reflect.Method;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.springframework.aop.MethodBeforeAdvice;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

public final class DaoEventDispatcher implements MethodBeforeAdvice, BeanFactoryAware {
    
    private static final int ON_DELETE = 1;
    
    private static final int ON_SAVE = 2;
    
    private int m_eventType;
    
    private ListableBeanFactory m_beanFactory;
    
    
    private DaoEventDispatcher(int eventType) {
        m_eventType = eventType;
    }
    
    /**
     * This can only be used withy listeable bean factory
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
    
    public BeanFactory getBeanFactory() {
        return m_beanFactory;
    }
    
    public static DaoEventDispatcher createDeleteDispatcher() {
        return new DaoEventDispatcher(ON_DELETE);
    }

    public static DaoEventDispatcher createSaveDispatcher() {
        return new DaoEventDispatcher(ON_SAVE);
    }

    public void before(Method m_, Object[] args, Object target_) throws Throwable {
        Map beanMap = m_beanFactory.getBeansOfType(DaoEventListener.class, true, true);
        onEvent(args, beanMap.values());
    }
    
    void onEvent(Object[] args, Collection listeners) {
        if (args.length == 0) {
            // empty arg calls like save() or delete() won't be considered
            return;
        }
        Iterator beans = listeners.iterator();
        while (beans.hasNext()) {
            DaoEventListener listener = (DaoEventListener) beans.next();
            switch (m_eventType) {
            case ON_DELETE:
                listener.onDelete(args[0]);
                break;
            case ON_SAVE:
                listener.onSave(args[0]);
                break;
            default:
                throw new RuntimeException("Unknown event type " + m_eventType);
            }
        }
    }    
}
