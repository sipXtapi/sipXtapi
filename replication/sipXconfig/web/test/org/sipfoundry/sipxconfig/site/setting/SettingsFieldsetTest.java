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

import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class SettingsFieldsetTest extends TestCase {

    private SettingsFieldset m_fieldset;

    protected void setUp() throws Exception {
        AbstractInstantiator instantiator = new AbstractInstantiator();
        m_fieldset = (SettingsFieldset) instantiator.getInstance(SettingsFieldset.class);
    }

    public void testRender() throws Exception {
        MockControl control = MockControl.createControl(Setting.class);
        Setting setting = (Setting) control.getMock();
        control.expectAndReturn(setting.isHidden(), false, 4);
        control.expectAndReturn(setting.isAdvanced(), true);
        control.expectAndReturn(setting.isAdvanced(), false);
        control.expectAndReturn(setting.isAdvanced(), true);
        control.expectAndReturn(setting.isAdvanced(), false);
        control.replay();

        m_fieldset.setShowAdvanced(true);
        assertTrue(m_fieldset.renderSetting(setting));
        assertTrue(m_fieldset.renderSetting(setting));

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.renderSetting(setting));
        assertTrue(m_fieldset.renderSetting(setting));

        control.verify();
    }

    public void testRenderHidden() throws Exception {
        MockControl control = MockControl.createControl(Setting.class);
        Setting setting = (Setting) control.getMock();
        control.expectAndReturn(setting.isHidden(), true, 2);
        control.replay();

        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.renderSetting(setting));

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.renderSetting(setting));

        control.verify();
    }

    public void testRenderSettingPlaceholder() throws Exception {
        MockControl control = MockControl.createControl(Setting.class);
        Setting setting = (Setting) control.getMock();
        control.expectAndReturn(setting.isAdvanced(), true, 2);
        control.expectAndReturn(setting.isAdvanced(), false, 2);
        control.replay();

        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.renderSettingPlaceholder(setting));

        m_fieldset.setShowAdvanced(false);
        assertTrue(m_fieldset.renderSettingPlaceholder(setting));

        m_fieldset.setShowAdvanced(true);
        assertFalse(m_fieldset.renderSettingPlaceholder(setting));

        m_fieldset.setShowAdvanced(false);
        assertFalse(m_fieldset.renderSettingPlaceholder(setting));

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
