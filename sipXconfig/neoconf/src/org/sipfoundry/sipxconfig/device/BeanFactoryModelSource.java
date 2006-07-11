/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Loads phone models from bean factory BeanFactoryPhoneModelSource
 */
public class BeanFactoryModelSource implements ModelSource, BeanFactoryAware {

    private ListableBeanFactory m_beanFactory;
    private Collection<PhoneModel> m_modelCache;
    private String m_prefix;

    public Collection<PhoneModel> getModels() {
        if (m_modelCache == null) {
            if (m_beanFactory == null) {
                throw new IllegalStateException("Bean factory has to be initialized");
            }
            m_modelCache = loadModels();
        }
        return m_modelCache;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        // invalidate cache
        m_modelCache = null;
    }

    /**
     * Sets the prefix that will be used to locate interesting beans.
     * 
     * Gateway model prefix is "gm", phone model prefix is "org.sipfoundry.sipxconfig.phone.".
     * TODO: change phone prefix to just "phone" or "pm"
     * 
     */
    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    private Collection<PhoneModel> loadModels() {
        String[] beanNames = m_beanFactory.getBeanNamesForType(PhoneModel.class);
        Collection<PhoneModel> models = new ArrayList<PhoneModel>(beanNames.length);
        for (String beanName : beanNames) {
            if (beanName.startsWith(m_prefix)) {
                PhoneModel bean = (PhoneModel) m_beanFactory.getBean(beanName);
                models.add(bean);
            }
        }
        return models;
    }
}
