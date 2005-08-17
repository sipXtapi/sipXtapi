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
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;
import java.util.Iterator;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.EmptyInterceptor;
import org.hibernate.EntityMode;
import org.hibernate.SessionFactory;
import org.hibernate.metadata.ClassMetadata;
import org.hibernate.type.Type;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Enables Spring to create hibernate object. Use when to allow Spring manage object dependencies
 * with hibernate All Interceptor methods, with the exception od instantiate, copied from
 * EmptyInterceptor
 */
public class SpringHibernateInstantiator extends EmptyInterceptor implements BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(SpringHibernateInstantiator.class);

    private ListableBeanFactory m_beanFactory;
    
    private SessionFactory m_sessionFactory;

    /**
     * This implementation only supports BeanWithId objects with integer ids
     */
    public Object instantiate(String entityName, EntityMode entityMode, Serializable id) {
        ClassMetadata classMetadata = m_sessionFactory.getClassMetadata(entityName);
        Class clazz = classMetadata.getMappedClass(entityMode);
        return instantiate(clazz, id);
    }

    Object instantiate(Class clazz, Serializable id) {
        String[] beanDefinitionNames = m_beanFactory.getBeanNamesForType(clazz);
        LOG.debug(beanDefinitionNames.length + " beans registered for class: " + clazz.getName());
        for (int i = 0; i < beanDefinitionNames.length; i++) {
            BeanWithId bean = (BeanWithId) m_beanFactory.getBean(beanDefinitionNames[i],
                    BeanWithId.class);
            // only return the bean if class matches exactly (do not return subclasses
            if (clazz == bean.getClass()) {
                bean.setId((Integer) id);
                return bean;
            }
        }
        return null;
    }

    /**
     * This can only be used withy listeable bean factory
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    /**
     * @see org.hibernate.Interceptor#onDelete(Object, Serializable, Object[], String[],
     *      Type[])
     */
    public void onDelete(Object entity_, Serializable id_, Object[] state_,
            String[] propertyNames_, Type[] types_) {
    }

    /**
     * @see org.hibernate.Interceptor#onFlushDirty(Object, Serializable, Object[], Object[],
     *      String[], Type[])
     */
    public boolean onFlushDirty(Object entity_, Serializable id_, Object[] currentState_,
            Object[] previousState_, String[] propertyNames_, Type[] types_) {
        return false;
    }

    /**
     * @see org.hibernate.Interceptor#onLoad(Object, Serializable, Object[], String[], Type[])
     */
    public boolean onLoad(Object entity_, Serializable id_, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        return false;
    }

    /**
     * @see org.hibernate.Interceptor#onSave(Object, Serializable, Object[], String[], Type[])
     */
    public boolean onSave(Object entity_, Serializable id_, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        return false;
    }

    /**
     * @see org.hibernate.Interceptor#postFlush(Iterator)
     */
    public void postFlush(Iterator entities_) {
    }

    /**
     * @see org.hibernate.Interceptor#preFlush(Iterator)
     */
    public void preFlush(Iterator entities_) {
    }

    /**
     * @see org.hibernate.Interceptor#findDirty(Object, Serializable, Object[], Object[],
     *      String[], Type[])
     */
    public int[] findDirty(Object entity_, Serializable id_, Object[] currentState_,
            Object[] previousState_, String[] propertyNames_, Type[] types_) {
        return null;
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }
    
}
