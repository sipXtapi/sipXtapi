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

import junit.framework.TestCase;

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class PhoneContextTest extends TestCase {
    
    public void testInstantiation() {        
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest");
        // now use some bean from factory
        PhoneContext phoneContext = (PhoneContext) bf.getFactory().getBean("phoneContext");
        assertNotNull(phoneContext);
        
        // stub implementation
        assertNotNull(phoneContext.getPhoneDao());
   }
}
