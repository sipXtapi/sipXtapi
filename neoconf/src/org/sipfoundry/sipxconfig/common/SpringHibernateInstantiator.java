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
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Enables Spring to create hibernate object. Use when to allow Spring manage object dependencies
 * with hibernate All Interceptor methods, with the exception od instantiate, copied from
 * EmptyInterceptor
 */
public class SpringHibernateInstantiator extends DaoEventDispatcher implements BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(SpringHibernateInstantiator.class);

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

    public void setSessionFactory(SessionFactory sessionFactory) {
        m_sessionFactory = sessionFactory;
    }    
}
