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

import java.io.StringWriter;
import java.io.Writer;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class Tp260GatewayTest extends TestCase {
    private AudioCodesModel m_model;
    private Tp260Gateway m_gateway;

    protected void setUp() throws Exception {
        m_model = (AudioCodesModel) TestHelper.getApplicationContext().getBean(
                "gmAudiocodesTP260_2_Span");
        m_gateway = (Tp260Gateway) TestHelper.getApplicationContext()
                .getBean("gwAudiocodesTp260");
        m_gateway.setBeanId(m_model.getBeanId());
        m_gateway.setModelId(m_model.getModelId());
    }

    public void testGenerateProfiles() throws Exception {
        assertSame(m_model, m_gateway.getModel());

        Writer writer = new StringWriter();
        m_gateway.generateProfiles(writer);

        // cursory check for now
        assertTrue(writer.toString().indexOf("VoiceVolume") >= 0);
    }

    public void testPrepareSettings() throws Exception {
        assertSame(m_model, m_gateway.getModel());

        MockControl defaultsCtrl = MockClassControl.createControl(PhoneDefaults.class);
        PhoneDefaults defaults = (PhoneDefaults) defaultsCtrl.getMock();
        defaults.getDomainName();
        defaultsCtrl.setDefaultReturnValue("mysipdomain.com");
        defaults.getProxyServerAddr();
        defaultsCtrl.setReturnValue("10.1.2.3");

        defaultsCtrl.replay();

        m_gateway.setDefaults(defaults);

        assertEquals("10.1.2.3", m_gateway.getSettingValue("SIPgw/ProxyIp"));
        assertEquals("mysipdomain.com", m_gateway.getSettingValue("SIPgw/ProxyName"));

        defaultsCtrl.verify();
    }

    public void testGetSettings() throws Exception {
        Setting settings = m_gateway.getSettings();
        assertEquals(new Integer(1), settings.getSetting("SIPgw/VoiceVolume").getTypedValue());
        assertTrue(settings instanceof SettingSet);
        SettingSet root = (SettingSet) settings;
        SettingSet currentSettingSet = (SettingSet) root.getDefaultSetting(SettingSet.class);
        assertEquals(Boolean.TRUE, currentSettingSet.getSetting("IsFaxUsed").getTypedValue());
    }
}
