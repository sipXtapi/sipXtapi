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

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class PhoneTestHelper {

    public static PhoneContext getPhoneContext() {
        return getPhoneContext("unittest");
    }

    public static PhoneContext getPhoneContextWithDb() {
        return getPhoneContext("unittest-db");
    }
    
    private static PhoneContext getPhoneContext(String factory) {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory(factory);
        // now use some bean from factory
        return (PhoneContext) bf.getFactory().getBean("phoneContext");        
    }
}