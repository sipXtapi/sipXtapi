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
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.hibernate.cfg.Configuration;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.orm.hibernate3.LocalSessionFactoryBean;

public class DynamicSessionFactoryBean extends LocalSessionFactoryBean implements
        BeanFactoryAware {
    public static final String HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "<!DOCTYPE hibernate-mapping PUBLIC \"-//Hibernate/Hibernate Mapping DTD 3.0//EN\" "
            + "   \"http://hibernate.sourceforge.net/hibernate-mapping-3.0.dtd\">";
    public static final String MAPPING_PATTERN = "<hibernate-mapping default-lazy=\"false\">"
            + "<subclass name=\"{0}\" extends=\"{1}\" discriminator-value=\"{2}\"/>"
            + "</hibernate-mapping>";

    private ListableBeanFactory m_beanFactory;

    /**
     * Collection of bean IDs that representing superclasses of the hierarchy. All beans of the
     * same type as bean ID will be automatically mapped to the same hibernate table.
     */
    private List m_baseClassBeanIds = Collections.EMPTY_LIST;

    protected void postProcessConfiguration(Configuration config) {
        for (Iterator i = m_baseClassBeanIds.iterator(); i.hasNext();) {
            String baseClassBeanID = (String) i.next();
            bindSubclasses(config, baseClassBeanID);
        }
    }

    /**
     * Finds all subclasesses of baseClass in the bean factory and binds them to the same table as
     * base class using bean id as a discriminator value.
     * 
     * @param config hibernate config that will be modified
     * @param baseClass base class - needs to be already mapped statically
     */
    protected void bindSubclasses(Configuration config, Class baseClass) {
        String[] beanDefinitionNames = m_beanFactory.getBeanNamesForType(baseClass);
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
     * Finds all subclasesses of baseClass in the bean factory and binds them to the same table as
     * base class using bean id as a discriminator value.
     * 
     * @param config hibernate config that will be modified
     * @param baseClassBeanId - bean representing the base class - needs to be already mapped
     *        statically
     */
    protected void bindSubclasses(Configuration config, String baseClassBeanId) {
        Class baseClass = m_beanFactory.getType(baseClassBeanId);
        String[] beanDefinitionNames = m_beanFactory.getBeanNamesForType(baseClass);
        for (int i = 0; i < beanDefinitionNames.length; i++) {
            String beanId = beanDefinitionNames[i];
            if (beanId.equals(baseClassBeanId)) {
                continue; // skip base class bean needs to be already mapped
            }
            Class subClass = m_beanFactory.getType(beanId);
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

    public void setBaseClassBeanIds(List baseClassBeanIds) {
        m_baseClassBeanIds = baseClassBeanIds;
    }
}