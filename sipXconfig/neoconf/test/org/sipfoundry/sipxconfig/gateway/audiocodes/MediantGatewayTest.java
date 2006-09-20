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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.File;
import java.io.StringWriter;
import java.io.Writer;

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class MediantGatewayTest extends TestCase {
    private AudioCodesModel m_model;
    private MediantGateway m_gateway;

    protected void setUp() throws Exception {
        m_model = (AudioCodesModel) TestHelper.getApplicationContext().getBean(
                "gmAudiocodesMP1X4_4_FXO");
        m_gateway = (MediantGateway) TestHelper.getApplicationContext().getBean(
                "gwAudiocodesMediant");
        m_gateway.setBeanId(m_model.getBeanId());
        m_gateway.setModelId(m_model.getModelId());
    }

    /**
     * Every test case must have 1 test 
     */
    public void testOnlyExistsBecauseAllOtherTestsDisabled() {        
    }

    /**
     * Disabled because hits the database
     */
    public void DISABLED_testGenerateProfiles() throws Exception {
        assertSame(m_model, m_gateway.getModel());

        Writer writer = new StringWriter();
        m_gateway.generateProfiles(writer);

        // cursory check for now
        assertTrue(writer.toString().indexOf("MAXDIGITS") >= 0);
    }

    /**
     * Disabled because hits the database
     */
    public void DISABLED_testGenerateAndRemoveProfiles() throws Exception {
        File file = m_gateway.getIniFile();
        m_gateway.generateProfiles();
        assertTrue(file.exists());
        m_gateway.removeProfiles();
        assertFalse(file.exists());
    }

    /**
     * Disabled because hits the database
     */
    public void DISABLED_testPrepareSettings() throws Exception {
        assertSame(m_model, m_gateway.getModel());

        IMocksControl defaultsCtrl = EasyMock.createControl();
        DeviceDefaults defaults = defaultsCtrl.createMock(DeviceDefaults.class);
        defaults.getDomainName();
        defaultsCtrl.andReturn("mysipdomain.com").anyTimes();
        defaults.getProxyServerAddr();
        defaultsCtrl.andReturn("10.1.2.3").atLeastOnce();

        defaultsCtrl.replay();

        m_gateway.setDefaults(defaults);

        assertEquals("10.1.2.3", m_gateway.getSettingValue("SIP_Params/PROXYIP"));
        assertEquals("mysipdomain.com", m_gateway.getSettingValue("SIP_Params/PROXYNAME"));

        defaultsCtrl.verify();
    }

    /**
     * Disabled because hits the database
     */
    public void DISABLED_testGetSettings() throws Exception {
        Setting settings = m_gateway.getSettings();
        assertEquals("15", settings.getSetting("SIP_Params/MAXDIGITS").getValue());
        assertTrue(settings instanceof SettingSet);
        SettingSet root = (SettingSet) settings;
        SettingSet currentSettingSet = (SettingSet) root.getSetting("SIP_Params");
        assertEquals("15", currentSettingSet.getSetting("MAXDIGITS").getValue());
    }
}
