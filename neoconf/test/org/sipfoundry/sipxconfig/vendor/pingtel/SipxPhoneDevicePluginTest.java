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
package org.sipfoundry.sipxconfig.vendor.pingtel;

import java.util.Iterator;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.core.DevicePlugin;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * Comments
 */
public class SipxPhoneDevicePluginTest extends TestCase {
    
    SipxConfig m_sipx;

    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();
        ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext(
            "org/sipfoundry/sipxconfig/vendor/pingtel/applicationContext-pingtel.xml");
        m_sipx = (SipxConfig) ctx.getBean("sipxconfig");
        assertNotNull(m_sipx);
    }

    /**
     * Doesn't really check values, just that they're not null. This 
     * may want to change
     */
    public void testGetters() {
        
        String macAddress = "00D001E0064CF";
        int expectedProfileCount = 4;
        
        Iterator pluginIds = m_sipx.getPluginIds().iterator();
        while (pluginIds.hasNext()) {
            DevicePlugin plugin = m_sipx.getDevicePlugin((String)pluginIds.next());
            assertNotNull(plugin.getDefinitions());
            
            assertEquals(plugin.getProfileCount(), expectedProfileCount);     
            for (int i = 0; i < expectedProfileCount; i++) {
                assertNotNull(plugin.getProfileFileName(i, macAddress));
                assertNotNull(plugin.getProfileSubscribeToken(i));
            }
        }
    }
}
