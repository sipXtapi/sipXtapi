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
    
    private final static String PHONE_CONTEXT = "phoneContext";
    
    private final static String DB_FACTORY_CONFIG = "beanRefFactory-db.xml";
    
    private final static String FACTORY_CONFIG = "beanRefFactory.xml";
    
    
    public static PhoneContext getPhoneContext() {
        return (PhoneContext) getFactory(FACTORY_CONFIG).getBean(PHONE_CONTEXT);
    }

    public static PhoneContext getPhoneContextWithDb() {
        return (PhoneContext) getFactory(DB_FACTORY_CONFIG).getBean(PHONE_CONTEXT);
    }
    
    public static UnitTestDao getUnitTestDao() {
        return (UnitTestDao) getFactory(DB_FACTORY_CONFIG).getBean("testData");
    }
    
    private static BeanFactory getFactory(String factory) {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance(factory);
        return  bfl.useBeanFactory("unittest").getFactory();
    }
}