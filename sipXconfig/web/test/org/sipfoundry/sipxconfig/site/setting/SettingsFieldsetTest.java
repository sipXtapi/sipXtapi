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
package org.sipfoundry.sipxconfig.site.setting;

import junit.framework.TestCase;

import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class SettingsFieldsetTest extends TestCase {

    private SettingsFieldset m_fieldset;

    protected void setUp() throws Exception {
        Creator instantiator = new Creator();
        m_fieldset = (SettingsFieldset) instantiator.newInstance(SettingsFieldset.class);
        m_fieldset.setSettings(new SettingSet("x"));
    }

    public void testRender() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        Setting setting = control.createMock(Setting.class);
        setting.getParent();
        control.andReturn(null).atLeastOnce();
        setting.isHidden();
        control.andReturn(false);
        setting.isAdvanced();
        control.andReturn(true);
        setting.isAdvanced();
        control.andReturn(false);
        setting.isAdvanced();
        control.andReturn(true);
        setting.isAdvanced();
        control.andReturn(false);
        control.replay();

        m_fieldset.getSettings().addSetting(setting);

        m_fieldset.setShowAdvanced(true);
        m_fieldset.setCurrentSetting(setting);
        assertTrue(m_fieldset.getRenderSetting());
        assertTrue(m_fieldset.getRenderSetting());

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.getRenderSetting());
        assertTrue(m_fieldset.getRenderSetting());

        control.verify();
    }

    public void testRenderHidden() throws Exception {
        IMocksControl control = EasyMock.createControl();
        Setting setting = control.createMock(Setting.class);
        setting.isHidden();
        control.andReturn(true).times(2);
        control.replay();

        m_fieldset.setCurrentSetting(setting);
        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.getRenderSetting());

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.getRenderSetting());

        control.verify();
    }

    public void testRenderSettingPlaceholder() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        Setting setting = control.createMock(Setting.class);
        setting.getParent();
        control.andReturn(null).atLeastOnce();
        setting.isAdvanced();
        control.andReturn(true).times(2);
        setting.isAdvanced();
        control.andReturn(false).times(2);
        control.replay();

        m_fieldset.getSettings().addSetting(setting);
        m_fieldset.setCurrentSetting(setting);
        
        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.getRenderSettingPlaceholder());

        m_fieldset.setShowAdvanced(false);
        assertTrue(m_fieldset.getRenderSettingPlaceholder());

        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.getRenderSettingPlaceholder());

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.getRenderSettingPlaceholder());

        control.verify();
    }

    public void testRenderToggle() throws Exception {
        SettingSet set = new SettingSet();

        SettingImpl toggle = new SettingImpl("toggle");

        set.addSetting(new SettingImpl("a"));
        set.addSetting(toggle);
        set.addSetting(new SettingImpl("c"));

        m_fieldset.setSettings(set);

        assertFalse(m_fieldset.getHasAdvancedSettings());
        toggle.setAdvanced(true);
        assertTrue(m_fieldset.getHasAdvancedSettings());
    }
}
