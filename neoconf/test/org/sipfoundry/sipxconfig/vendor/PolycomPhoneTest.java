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

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.SettingSet;
import org.sipfoundry.sipxconfig.settings.NetworkSettings;
import org.sipfoundry.sipxconfig.settings.PhoneSettings;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class PolycomPhoneTest extends TestCase {
    
    private PhoneContext m_phoneContext;
    
    public void setUp() {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest");
        // now use some bean from factory 
        m_phoneContext = (PhoneContext) bf.getFactory().getBean("phoneContext");
        assertNotNull(m_phoneContext);               
    }
    
    public void testSave() {
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        Phone phone = m_phoneContext.getPhone(endpoint);
        assertNotNull(phone);
        
        PhoneSettings settings = (PhoneSettings) phone.getSettings(new SettingSet());
        NetworkSettings network = settings.getNetworkSettings();
        network.getSetting("tftpServer").setValue("hey there");
    }

}
