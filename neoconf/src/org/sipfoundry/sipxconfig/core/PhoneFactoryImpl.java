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
package org.sipfoundry.sipxconfig.core;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

/**
 * Comments
 */
public class PhoneFactoryImpl implements BeanFactoryAware, PhoneFactory {

    private BeanFactory m_beanFactory;

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public Phone getPhoneById(String id, Endpoint endpoint) {
        Phone phone = (Phone) m_beanFactory.getBean(id);
        phone.setEndpoint(endpoint);
        
        return phone;
    }
}
