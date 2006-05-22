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

import java.util.Stack;

public class BeanWithSettingsModel implements SettingModel2 {        
    private Stack<SettingValueHandler> m_defaultHandlers = new Stack();    
    private SettingValueHandler m_defaultsHandler = new MulticastSettingValueHandler(m_defaultHandlers);    
    private ProfileNameHandler m_defaultProfileNameHandler;    
    private BeanWithSettings m_bean;
    
    BeanWithSettingsModel(BeanWithSettings bean) {
        m_bean = bean;
    }
    
    public void addDefaultsHandler(SettingValueHandler handler) {
        m_defaultHandlers.push(handler);
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

    public SettingValue2 getSettingValue(Setting setting, SettingValue2 originalValue) {
        SettingValue2 value = null;
        Storage vs = getBeanWithSettings().getValueStorage();
        if (vs != null) {
            value = vs.getSettingValue(setting);
        }
        
        if (value == null) {
            value = getDefault(setting);
        }
        
        if (value == null) {
            value = originalValue;
        }
        
        return value;
    }
    
    public SettingValue2 getDefaultSettingValue(Setting setting, SettingValue2 originalValue) {
        // just do not consult the bean, that is the default
        SettingValue2 lvalue = getDefault(setting);
        return lvalue != null ? lvalue : originalValue;
    }        
    
    protected SettingValue2 getDefault(Setting setting) {
        SettingValue2 value = getDefaultsHandler().getSettingValue(setting);
        return value;
    }
        
    /**
     * not pretty, but need to populate setting model into every setting instance
     */
    static class SetModelReference implements SettingVisitor {
        private SettingModel2 m_model;
        public SetModelReference(SettingModel2 model) {
            m_model = model;
        }

        public void visitSetting(Setting setting) {
            setReference(setting);
        }

        public void visitSettingGroup(Setting group) {
        }
        
        private void setReference(Setting s) {
            SettingImpl impl = SettingUtil.getSettingImpl(s);
            impl.setModel(m_model);
        }
    }

    public void setSettingValue(Setting setting, String sValue) {
        SettingValue2 defaultValue = new SettingValueImpl(setting.getDefaultValue());
        SettingValue2 value = new SettingValueImpl(sValue);
        Storage vs = m_bean.getInitializeValueStorage();       
        vs.setSettingValue(setting, value, defaultValue);        
    }

    public SettingValue2 getProfileName(Setting setting, SettingValue2 originalValue) {
        SettingValue2 value = originalValue;
        if (m_defaultProfileNameHandler != null) {
            SettingValue2 lvalue = m_defaultProfileNameHandler.getProfileName(setting);
            if (lvalue != null) {
                lvalue = value;
            }
        }

        return value;
    }
}
