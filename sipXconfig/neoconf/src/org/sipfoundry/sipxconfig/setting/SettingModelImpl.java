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
    
    public void addSettingValueHandler(SettingValueHandler handler) {
        m_handlers.push(handler);
    }
    
    public String getSettingValue(String path) {
        SettingValue2 value = m_multicast.getSettingValue(path);        
        return value != null ? value.getValue() : null;
    }
}
