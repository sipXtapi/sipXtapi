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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.hibernate.EntityMode;
import org.hibernate.SessionFactory;
import org.hibernate.metadata.ClassMetadata;
import org.sipfoundry.sipxconfig.search.IndexingInterceptor;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Enables Spring to create the hibernate object. Use to allow Spring to manage object
 * dependencies with hibernate.
 * 
 * Note: it inherits from IndexingInterceptor: only one interceptor can be registered with
 * hibernate session.
 */
public class SpringHibernateInstantiator extends IndexingInterceptor implements BeanFactoryAware {
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
        ListableBeanFactory beanFactory = (ListableBeanFactory) getBeanFactory();
        String[] beanDefinitionNames = beanFactory.getBeanNamesForType(clazz);
        LOG.debug(beanDefinitionNames.length + " beans registered for class: " + clazz.getName());
        for (int i = 0; i < beanDefinitionNames.length; i++) {
            BeanWithId bean = (BeanWithId) beanFactory.getBean(beanDefinitionNames[i],
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

    public BeanFactory getBeanFactory() {
        return m_beanFactory;
    }

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }
}
