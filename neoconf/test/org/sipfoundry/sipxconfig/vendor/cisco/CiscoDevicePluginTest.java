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
package org.sipfoundry.sipxconfig.vendor.cisco;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.core.DevicePlugin;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public class CiscoDevicePluginTest extends TestCase {
    
    private DevicePlugin m_7960;

    public void setUp() {
        ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext(
                "org/sipfoundry/sipxconfig/vendor/cisco/applicationContext-cisco.xml");
        SipxConfig sipx = (SipxConfig) ctx.getBean("sipxconfig");
        assertNotNull(sipx);

        m_7960 = sipx.getDevicePlugin(CiscoDevicePlugin.MODEL_7960);
        assertTrue(m_7960 != null);
    }

    public void testGetters() {
        assertNotNull(m_7960.getDefinitions());
        String macAddress = "00D001E0064CF";        
        assertEquals(m_7960.getProfileCount(), 1);                 
        assertNotNull(m_7960.getProfileFileName(1, macAddress));
        try
        {
            // not supported
            assertNotNull(m_7960.getProfileSubscribeToken(1));
            fail();
        } catch (IllegalArgumentException e)
        {
            assertTrue(true);
        }
    }
}
