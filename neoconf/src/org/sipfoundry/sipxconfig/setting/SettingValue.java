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



/**
 * Proxy setting class thru a decorator that manages values in the ValueStorage
 * class. 
 */
public class SettingValue extends SettingDecorator {
    
    private ValueStorage m_valueStorage;
    
    public SettingValue() {        
        /// BEAN ACCESS ONLY
    }
    
    public SettingValue(ValueStorage valueStorage, Setting delegate) {
        super(delegate);
        m_valueStorage = valueStorage;
    }
    
    public String getDefaultValue() {
        return getDelegate().getValue();
    }
    
    public void setValue(String value) {
        String defValue = getDefaultValue();
        if (value == null) {
            if (defValue == null) {
                m_valueStorage.remove(getDelegate().getPath());
            } else {
                m_valueStorage.put(getDelegate().getPath(), Setting.NULL_VALUE);
            }
        } else {
            if (value.equals(defValue)) {
                m_valueStorage.remove(getDelegate().getPath());
            } else {
                m_valueStorage.put(getDelegate().getPath(), value);                
            }
        }
    }
    
    public String getValue() {
        String value = (String) m_valueStorage.get(getDelegate().getPath());        
        return value == null ? getDefaultValue() : value;
    }    
}
