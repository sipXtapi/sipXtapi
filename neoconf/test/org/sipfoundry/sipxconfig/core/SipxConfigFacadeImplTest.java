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
package org.sipfoundry.sipxconfig.core;

import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * Comments
 */
public class SipxConfigFacadeImplTest extends TestCase {
    
    private SipxConfigFacadeImpl m_facade;
    
    private SipxConfig m_sipx;
        
    private String m_macAddress = "00D001E0064CF";

    public void setUp() {
        ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext(
        	"org/sipfoundry/sipxconfig/core/applicationContext-device.xml");
        m_facade = (SipxConfigFacadeImpl)ctx.getBean("sipxfacade");
        assertNotNull(m_facade);
        
        m_sipx = (SipxConfig)ctx.getBean("sipxconfig");
        assertNotNull(m_sipx);        
    }
    
    public void testFacade()
    {
        MockControl pluginControl = MockControl.createNiceControl(DevicePlugin.class);
        DevicePlugin plugin = (DevicePlugin) pluginControl.getMock();
        pluginControl.expectAndReturn(plugin.getPluginId(), "MockPlugin");
        pluginControl.expectAndReturn(plugin.getProfileCount(), 1);
        pluginControl.expectAndReturn(plugin.getProfileFileName(0, m_macAddress), "profile.dat");
        pluginControl.expectAndReturn(plugin.getProfileSubscribeToken(0), "profile-token");
        pluginControl.replay();        
        
        Set plugins = new HashSet();
        plugins.add(plugin);
        m_sipx.setDevicePlugins(plugins);

        m_facade.getDeviceProfileName(0, "MockPlugin", "", m_macAddress);
        m_facade.getDeviceProfileToken(0, "MockPlugin");
        pluginControl.verify();
    }
}
