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
    }
    
    public void addSettingValueHandler(SettingValueHandler handler) {
        m_handlers.push(handler);
    }
    
    public String getSettingValue(String path) {
        Setting setting = m_settings.getSetting(path);
        SettingValue2 value = m_multicast.getSettingValue(setting);        
        return value != null ? value.getValue() : null;
    }
}
