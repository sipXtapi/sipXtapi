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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.core.CoreDao;
import org.sipfoundry.sipxconfig.core.Line;
import org.sipfoundry.sipxconfig.core.LogicalPhone;
import org.sipfoundry.sipxconfig.core.Organization;
import org.sipfoundry.sipxconfig.core.Phone;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.sipfoundry.sipxconfig.core.User;
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
                
        m_7960 = (Phone)m_sipx.getPhoneFactory().getPhoneById(CiscoPhone.MODEL_7960);
        assertTrue(m_7960 != null);
    }
    
    public void test7960Getters() {
        // Setup mock database calls
        MockControl daoControl = MockControl.createControl(CoreDao.class);
        CoreDao dao = (CoreDao) daoControl.getMock();
        Organization org = new Organization();
        org.setDnsDomain("domainname.com");
        User user = new User();
        user.setOrganization(org);
        Line line = new Line();
        line.setName("joe");
        LogicalPhone logicalPhone = new LogicalPhone();
        logicalPhone.setUser(user);
        daoControl.expectAndReturn(dao.loadLogicalPhone(1), logicalPhone);
        daoControl.expectAndReturn(dao.loadLine(user, 0), line);        
        daoControl.replay();
        m_sipx.setCoreDao(dao);
        
        assertNotNull(m_7960.getDefinitions());
        String macAddress = "00D001E0064CF";        
        assertEquals(m_7960.getProfileCount(), 1);                 
        assertNotNull(m_7960.getProfileFileName(1, macAddress));
        assertNull(m_7960.getProfileSubscribeToken(1));
        LogicalPhone phone = (LogicalPhone)m_sipx.getCoreDao().loadLogicalPhone(1);
        assertNotNull(phone);
        
        assertEquals("sip:joe@domainname.com", m_7960.getProfileNotifyUrl(phone, 0));        
        assertEquals(m_7960.getProfileSequenceNumber(phone, 0), 
                Phone.SEQUENCE_NUMBER_NOT_SUPPORTED);

        daoControl.verify();
    }
}
