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
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class PhoneTestHelper {
    
    private BeanFactory m_factory; 
    
    private final static String DB_FACTORY_CONFIG = "beanRefFactory-db.xml";
    
    private final static String FACTORY_CONFIG = "beanRefFactory.xml";

    private PhoneTestHelper(String factory) {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance(factory);
        m_factory = bfl.useBeanFactory("unittest").getFactory();
    }
    
    public static PhoneTestHelper createHelper() {
        return new PhoneTestHelper(FACTORY_CONFIG);
    }
    
    public static PhoneTestHelper createHelperWithDatabase() {
        return new PhoneTestHelper(DB_FACTORY_CONFIG);
    }
    
    public PhoneContext getPhoneContext() {
        return (PhoneContext) m_factory.getBean(PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public UnitTestDao getUnitTestDao() {
        return (UnitTestDao) getFactory().getBean("testData");
    }

    public BeanFactory getFactory() {
        return m_factory;
    }
}