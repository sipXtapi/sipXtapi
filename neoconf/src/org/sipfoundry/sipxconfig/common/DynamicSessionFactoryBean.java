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

import java.text.MessageFormat;

import net.sf.hibernate.HibernateException;
import net.sf.hibernate.MappingException;
import net.sf.hibernate.cfg.Configuration;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.orm.hibernate.LocalSessionFactoryBean;

public class DynamicSessionFactoryBean extends LocalSessionFactoryBean implements
        BeanFactoryAware {
    public static final String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "<!DOCTYPE hibernate-mapping PUBLIC \"-//Hibernate/Hibernate Mapping DTD//EN\" "
            + "   \"http://hibernate.sourceforge.net/hibernate-mapping-2.0.dtd\">";
    public static final String MAPPING_PATTERN = "<hibernate-mapping>"
            + "<subclass name=\"{0}\" extends=\"{1}\" discriminator-value=\"{2}\"/>"
            + "</hibernate-mapping>";

    private ListableBeanFactory m_beanFactory;

    protected void postProcessConfiguration(Configuration config) throws HibernateException {
        bindSubclasses(config, Gateway.class);
    }

    /**
     * Finds all subclasesses of baseClass in the bean factory and binds them to the same table as
     * base class using bean id as a discriminator value.
     * 
     * @param config hibernate config that will be modified
     * @param baseClass base class - needs to be already mapped statically
     * @throws MappingException
     */
    protected void bindSubclasses(Configuration config, Class baseClass) throws MappingException {
        String[] beanDefinitionNames = m_beanFactory.getBeanDefinitionNames(baseClass);
        for (int i = 0; i < beanDefinitionNames.length; i++) {
            String beanId = beanDefinitionNames[i];
            Class subClass = m_beanFactory.getType(beanId);
            if (subClass == baseClass) {
                continue; // skip baseclass which is already mapped
            }
            String mapping = xmlMapping(baseClass, subClass, beanId);
            config.addXML(mapping);
        }
    }

    /**
     * Create XML that contains a single subclass mapping
     * 
     * @param baseClass already mapped hibernate entity class
     * @param subClass new entity to be mapped
     * @param discriminator value of disciminator for this subclass
     * @return xml string that can be parsed by hibernate to add new mapping
     */
    String xmlMapping(Class baseClass, Class subClass, String discriminator) {
        String mapping = MessageFormat.format(MAPPING_PATTERN, new Object[] {
            subClass.getName(), baseClass.getName(), discriminator
        });
        return HEADER + mapping;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        if (!(beanFactory instanceof ListableBeanFactory)) {
            throw new BeanInitializationException(getClass()
                    + " only works with ListableBeanFactory");
        }
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
