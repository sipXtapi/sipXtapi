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
package org.sipfoundry.sipxconfig.settings;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

/**
 * Comments
 */
public class SettingsContextImpl implements BeanFactoryAware, SettingsContext {
    
    private SettingsDao m_settingsDao;

    private BeanFactory m_beanFactory;

    public SettingsDao getSettingsDao() {
        return m_settingsDao;
    }

    public void setSettingsDao(SettingsDao settingsDao) {
        m_settingsDao = settingsDao;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
