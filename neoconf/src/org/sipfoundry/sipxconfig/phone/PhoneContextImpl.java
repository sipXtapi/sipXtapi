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
package org.sipfoundry.sipxconfig.phone;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;


/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl implements BeanFactoryAware, PhoneContext {

    private PhoneDao m_dao;

    private BeanFactory m_beanFactory;

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
    
    public Phone getPhone(Endpoint endpoint) {
        Phone phone = (Phone) m_beanFactory.getBean(endpoint.getPhoneId());
        if (phone != null) {
            phone.setEndpoint(endpoint);
        }
        
        return phone;
    }
    
    public PhoneDao getPhoneDao() {
        return m_dao;
    }

    public void setPhoneDao(PhoneDao dao) {
        m_dao = dao;
    }    
}
