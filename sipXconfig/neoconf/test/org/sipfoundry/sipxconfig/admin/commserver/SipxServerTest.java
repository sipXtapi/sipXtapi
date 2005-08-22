/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServerTest extends TestCase {

    private SipxServer m_server;


    protected void setUp() throws Exception {
        m_server = new SipxServer();
        m_server.setConfigDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        m_server.setSettingModel(TestHelper.loadSettings("commserver/server.xml"));
    }    
    
    public void testGetSetting() {
        Setting settings = m_server.getSettings();
        assertNotNull(settings);
    }
    
    public void testGetServerSettings() {
        String expected = "myserver";
        m_server.getSettings().getSetting("domain/SIPXCHANGE_DOMAIN_NAME").setValue(expected);
        String actual = m_server.getServerSettings().getDomainName();
        assertNotNull(expected, actual);
    }
}
