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
import java.util.Set;

import org.apache.commons.collections.map.LinkedMap;

/**
 * Meta information about a group of settings, can contain nested
 * SettingModels.  Order is preserved
 */
public class SettingGroup extends Setting {
    
    private LinkedMap m_delegate = new LinkedMap();
        
    /** 
     * Root setting group and bean access only
     */
    public SettingGroup() {
    }
        
    public SettingGroup(String name) {
        super(name);
    }
    
    /**
     * generate a mutable, deep copy populated with the settings,  recursive
     */
    public Setting getCopy(ValueStorage valueStorage) {
        setValueStorage(valueStorage);
        SettingGroup clone = (SettingGroup) this.clone();
        int size = m_delegate.size();        
        clone.m_delegate = new LinkedMap(size > 0 ? size : 1); // throws error if ever size = 0        
        Iterator values = m_delegate.values().iterator();
        while (values.hasNext()) {
            Setting setting = (Setting) values.next();
            Setting settingCopy = setting.getCopy(valueStorage);
            clone.addSetting(settingCopy);
        }

        return clone;
    }
            
    /**
     * the nth item that was added to this model
     */
    public Setting getSetting(int index) {
        Object key = m_delegate.get(index);
        return (Setting) (key != null ? m_delegate.get(key) : null);
    }

    /**
     * adds the setting to this group collection along with setting the
     * group on the setting 
     */
    public Setting addSetting(Setting setting) {
        setting.setSettingGroup(this);
        m_delegate.put(setting.getName(), setting);
        
        return setting;
    }

    public Setting getSetting(String name) {
        return (Setting) m_delegate.get(name);
    }
    
    /**
     * M A P  I M P L E M E N T A T I O N
     * quasi implementation, turns out little benefit to support map interface
     * infact tapestry gets confused and all other methods do not get called
     * instean map.get(methodName) tends to get called.
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

    public boolean hasKey(Object key) {
        return m_delegate.containsKey(key);
    }

    public boolean hasValue(Object value) {
        return m_delegate.containsValue(value);
    }

    public Collection getValues() {
        return m_delegate.values();
    }

    public Set entrySet() {
        return m_delegate.entrySet();
    }

    public Set keySet() {
        return m_delegate.keySet();
    }

    public String dump() {
        StringBuffer dump = new StringBuffer();
        dump.append(super.dump());
        Iterator children = getValues().iterator();
        while (children.hasNext()) {
            dump.append("\n");
            dump.append(((Setting) (children.next())).dump());            
        }
        
        return dump.toString();
    }
}
