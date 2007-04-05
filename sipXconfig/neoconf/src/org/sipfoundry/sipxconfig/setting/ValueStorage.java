/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Basic layer of settings decoration that captures just setting values.
 */
public class ValueStorage extends BeanWithId implements Storage {

    private Map m_delegate = new HashMap();

    public Map getValues() {
        return m_delegate;
    }

    public void setValues(Map delegate) {
        m_delegate = delegate;
    }
    
    public int size() {
        return m_delegate.size();
    }    

    public SettingValue getSettingValue(Setting setting) {
        if (getValues() == null) {
            return null;
        }
        
        SettingValue settingValue = null;
        // null is legal value so test for key existance
        if (getValues().containsKey(setting.getPath())) {            
            String value = (String) getValues().get(setting.getPath());
            settingValue = new SettingValueImpl(value);
        }
        return settingValue;
    }

    public void setSettingValue(Setting setting, SettingValue value, SettingValue defaultValue) {
        if (value.equals(defaultValue)) {
            revertSettingToDefault(setting);
        } else {
            getValues().put(setting.getPath(), value.getValue());
        }
    }

    public void revertSettingToDefault(Setting setting) {
        getValues().remove(setting.getPath());
    }
}
