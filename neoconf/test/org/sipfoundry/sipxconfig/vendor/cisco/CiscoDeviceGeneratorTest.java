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

import org.sipfoundry.sipxconfig.core.DeviceModel;
import org.sipfoundry.sipxconfig.core.DevicePlugin;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.context.support.ClassPathXmlApplicationContext;

public class CiscoDeviceGeneratorTest extends TestCase {
    
    private DevicePlugin m_plugin;

    public void setUp()
    {
        ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext(
        	"org/sipfoundry/sipxconfig/vendor/cisco/applicationContext-cisco.xml");
        SipxConfig sipx = (SipxConfig) ctx.getBean("sipxconfig");
        assertNotNull(sipx);

        m_plugin =  (DevicePlugin)sipx.getDevicePlugins().get("cisco");
        assertTrue(m_plugin != null);
    }
    
    public void testGetPropertyDefitions()
    {
        DeviceModel[] models = m_plugin.getModels();
        for (int i = 0; i < models.length; i++) {
            assertNotNull(m_plugin.getGenerator(models[i]).getDefinitions());
        }
        
    }
}
