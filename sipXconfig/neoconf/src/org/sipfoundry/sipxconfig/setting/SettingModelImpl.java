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

public class SettingModelImpl implements SettingModel2 {
    private Stack<SettingValueHandler> m_handlers = new Stack();
    private MulticastSettingValueHandler m_multicast = new MulticastSettingValueHandler(m_handlers);
    private Setting m_settings;
    
    public void setSettings(Setting settings) {
        m_settings = settings;
        m_settings.acceptVisitor(new SetModelReference(this));
    }
    
    public void addSettingValueHandler(SettingValueHandler handler) {
        m_handlers.push(handler);
    }
    
    public String getSettingValue(Setting setting, String defaultValue) {
        SettingValue2 value = m_multicast.getSettingValue(setting);        
        return value == null ? defaultValue : value.getValue();
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
            setReference(group);
        }
        
        private void setReference(Setting s) {
            // HACK UNTIL DECORATORS ARE REMOVED
            if (s instanceof SettingImpl) {
                SettingImpl impl = (SettingImpl) s;
                impl.setModel(m_model);
            } else if (s instanceof SettingDecorator) {
                SettingDecorator sd = (SettingDecorator) s;
                Setting delegate = sd.getDelegate();                
                setReference(delegate);
            }
        }
    }
}
