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
package org.sipfoundry.sipxconfig.site;

import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

public class SiteTestHelper {
    
    private BeanFactory m_factory; 
    
    private final static String DB_FACTORY_CONFIG = "beanRefFactory-db.xml";
    
    private final static String FACTORY_CONFIG = "beanRefFactory.xml";

    private SiteTestHelper(String factory) {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance(factory);
        m_factory = bfl.useBeanFactory("unittest").getFactory();
        TapestryUtils.setUnittestBeanFactory(m_factory);
    }
    
    public static SiteTestHelper createHelper() {
        return new SiteTestHelper(FACTORY_CONFIG);
    }
    
    public static SiteTestHelper createHelperWithDatabase() {
        return new SiteTestHelper(DB_FACTORY_CONFIG);
    }
    
    public PhoneContext getPhoneContext() {
        return (PhoneContext) m_factory.getBean(PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public BeanFactory getFactory() {
        return m_factory;
    }
}
