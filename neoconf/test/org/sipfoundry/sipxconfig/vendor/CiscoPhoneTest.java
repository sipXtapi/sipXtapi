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

import org.sipfoundry.sipxconfig.core.LogicalPhone;
import org.sipfoundry.sipxconfig.core.Phone;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

public class CiscoPhoneTest extends TestCase {
    
    private Phone m_7960;
    
    private SipxConfig m_sipx;

    public void setUp() {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest");
        // now use some bean from factory 
        m_sipx = (SipxConfig) bf.getFactory().getBean("sipxconfig");
        assertNotNull(m_sipx);

        m_7960 = (Phone)m_sipx.getPhoneFactory().getBean(CiscoPhone.MODEL_7960);
        assertTrue(m_7960 != null);
    }

    public void test7960Getters() {
        assertNotNull(m_7960.getDefinitions());
        String macAddress = "00D001E0064CF";        
        assertEquals(m_7960.getProfileCount(), 1);                 
        assertNotNull(m_7960.getProfileFileName(1, macAddress));
        assertNull(m_7960.getProfileSubscribeToken(1));
        // TODO: load a test phone 
        //LogicalPhone phone = (LogicalPhone)m_sipx.getCoreDao().loadLogicalPhone(1);
        //assertNotNull(m_7960.getProfileNotifyUrl(phone, 0));
    }
}
