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

import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.map.LinkedMap;

/**
 * Meta information about a group of settings, can contain nested
 * SettingModels.  Order is preserved
 */
public class SettingGroup extends Setting {
    
    private LinkedMap m_delegate = new LinkedMap();
    
    private Map m_settingValues = new SettingMap();

    /**
     * Create the top most model
     */
    public SettingGroup() {
    }
    
    public SettingGroup(String name) {
        super(name);
    }
    
    /**
     * generate a mutable, deep copy populated with the settings,  recursive
     */
    public Setting deepClone() {
        SettingGroup clone = (SettingGroup) this.clone();
        int size = m_delegate.size();        
        clone.m_delegate = new LinkedMap(size > 0 ? size : 1); // throws error if ever size = 0        
        Iterator values = m_delegate.values().iterator();
        while (values.hasNext()) {
            Setting setting = (Setting) values.next();
            Setting settingCopy = setting.deepClone();
            clone.put(settingCopy.getName(), settingCopy);
        }

        return clone;
    }
    
    public SettingValue getSettingValue() {
        return (SettingValue) m_settingValues.get(getPath());
    }
    
    public void setSettingValue(SettingValue settingValue) {        
        if (settingValue.getValue() == null) {
            m_settingValues.remove(getPath());
        } else {            
            m_settingValues.put(getPath(), settingValue);
        }
    }

    /**
     * accumulate values the user has actually set.  will clear all previsous settings
     * 
     * @return null if no settings values
     */
    public Map getSettingValues() {
        return m_settingValues;
    }
    
    /**
     * The values the user has actually set
     * 
     * @return null if no settings values
     */
    public void setSettingValues(Map settingValues) {
        if (settingValues == null) {
            m_settingValues.clear();
        } else {
            m_settingValues = settingValues;
        }
        super.setSettingValues(m_settingValues);
        Iterator values = m_delegate.values().iterator();
        while (values.hasNext()) {
            ((Setting) values.next()).setSettingValues(m_settingValues);            
        }        
    }
    
    /**
     * the nth item that was added to this model
     */
    public Setting getSetting(int index) {
        Object key = m_delegate.get(index);
        return (Setting) (key != null ? m_delegate.get(key) : null);
    }

    /**
     * @throws IllegalArgumentException  Cannot put settings into another setting, only groups
     */
    public Setting addSetting(Setting setting) {
        setting.setSettingGroup(this);
        setting.setSettingValues(m_settingValues);
        m_delegate.put(setting.getName(), setting);
        
        return setting;
    }

    public Setting getSetting(String name) {
        return (Setting) m_delegate.get(name);
    }
    
    /**
     * M A P  I M P L E M E N T A T I O N
     */
    public int size() {
        return m_delegate.size();
    }

    public void clear() {
        m_delegate.clear();
    }

    public boolean isEmpty() {
        return m_delegate.isEmpty();
    }

    public boolean containsKey(Object key) {
        return m_delegate.containsKey(key);
    }

    public boolean containsValue(Object value) {
        return m_delegate.containsValue(value);
    }

    public Collection values() {
        return m_delegate.values();
    }

    public void putAll(Map t) {
        m_delegate.putAll(t);
    }

    public Set entrySet() {
        return m_delegate.entrySet();
    }

    public Set keySet() {
        return m_delegate.keySet();
    }

    public Object get(Object key) {
        return m_delegate.get(key);
    }

    public Object remove(Object key) {
        return m_delegate.remove(key);
    }

    public Object put(Object key_, Object value) {
        return addSetting((Setting) value);
    }    
}
