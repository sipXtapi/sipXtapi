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
package org.sipfoundry.sipxconfig.vendor;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class PolycomPhoneTest extends TestCase {
    
    private SipxConfig m_sipx;
    
    public void setUp() {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest");
        // now use some bean from factory 
        m_sipx = (SipxConfig) bf.getFactory().getBean("sipxconfig");
        assertNotNull(m_sipx);                
    }
    
    public void testSave() {
        /*
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        Phone phone = m_sipx.getPhoneFactory().getPhoneById(endpoint);
        assertNotNull(phone);
        
        PhonePropertyEditor editor = phone.getPropertyEditor();
        editor.getProperty("TFTPBootServer").setValue("localhost");
        editor.saveAllProperties();
        */
    }

}
