/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.List;

public class BeanWithSettingsModel implements SettingModel2 {
    private List<SettingValueHandler> m_defaultHandlers = new ArrayList<SettingValueHandler>();
    private SettingValueHandler m_defaultsHandler = new MulticastSettingValueHandler(
            m_defaultHandlers);
    private ProfileNameHandler m_defaultProfileNameHandler;
    private BeanWithSettings m_bean;

    BeanWithSettingsModel(BeanWithSettings bean) {
        m_bean = bean;
    }

    public void addDefaultsHandler(SettingValueHandler handler) {
        m_defaultHandlers.add(handler);
    }

    public void setDefaultProfileNameHandler(ProfileNameHandler handler) {
        m_defaultProfileNameHandler = handler;
    }

    public void setSettings(Setting settings) {
        if (settings != null) {
            settings.acceptVisitor(new SetModelReference(this));
        }
    }

    protected BeanWithSettings getBeanWithSettings() {
        return m_bean;
    }

    protected SettingValueHandler getDefaultsHandler() {
        return m_defaultsHandler;
    }

    public SettingValue2 getSettingValue(Setting setting) {
        SettingValue2 value = null;
        Storage vs = getBeanWithSettings().getValueStorage();
        if (vs != null) {
            value = vs.getSettingValue(setting);
        }

        if (value == null) {
            value = getDefault(setting);
        }

        return value;
    }

    public SettingValue2 getDefaultSettingValue(Setting setting) {
        // just do not consult the bean, that is the default
        return getDefault(setting);
    }

    protected SettingValue2 getDefault(Setting setting) {
        SettingValue2 value = getDefaultsHandler().getSettingValue(setting);
        return value;
    }

    /**
     * not pretty, but need to populate setting model into every setting instance
     */
    static class SetModelReference extends AbstractSettingVisitor {
        private SettingModel2 m_model;

        public SetModelReference(SettingModel2 model) {
            m_model = model;
        }

        public void visitSetting(Setting setting) {
            SettingImpl impl = SettingUtil.getSettingImpl(setting);
            impl.setModel(m_model);
        }
    }

    public void setSettingValue(Setting setting, String sValue) {
        SettingValue2 defaultValue = new SettingValueImpl(setting.getDefaultValue());
        SettingValue2 value = new SettingValueImpl(sValue);
        Storage vs = m_bean.getInitializeValueStorage();
        vs.setSettingValue(setting, value, defaultValue);
    }

    public SettingValue2 getProfileName(Setting setting) {
        if (m_defaultProfileNameHandler != null) {
            return m_defaultProfileNameHandler.getProfileName(setting);
        }
        return null;
    }
}
