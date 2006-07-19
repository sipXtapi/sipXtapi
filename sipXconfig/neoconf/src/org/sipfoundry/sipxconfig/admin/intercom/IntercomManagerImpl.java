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
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class IntercomManagerImpl
    extends SipxHibernateDaoSupport
    implements IntercomManager, BeanFactoryAware {

    public static final String CONTEXT_BEAN_NAME = "intercomManagerImpl";
    
    private BeanFactory m_beanFactory;

    public Intercom newIntercom() {
        return (Intercom) m_beanFactory.getBean(Intercom.class.getName());
    }

    public void saveIntercom(Intercom intercom) {
        getHibernateTemplate().saveOrUpdate(intercom);
    }

    public List loadIntercoms() {
        return getHibernateTemplate().loadAll(Intercom.class);
    }

    /**
     * Remove all intercoms - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection intercoms = template.loadAll(Intercom.class);
        template.deleteAll(intercoms);
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
