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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Meta Information about a Setting.
 */
public class Setting implements Map, Cloneable {

    private String m_label;

    private String m_type;

    private String m_name;
    
    private SettingValue m_settingValue;

    private String m_defaultValue;

    private List m_possibleValues;
    
    private SettingGroup m_settingGroup;
    
    /**
     * bean access only, must set name before valid object
     */
    public Setting() {
    }
    
    public Setting(String name) {
        setName(name);
    }
    
    public Setting(String name, String value) {
        this(name);
        setValue(value);
    }

    public Setting deepClone() {
        return (Setting) clone();        
    }
        
    public void setSettingValues(Map settingValues) {
        m_settingValue = (SettingValue) settingValues.get(getPath());
    }
    
    void updateSettingValues() {
        if (m_settingGroup != null) {
            if (m_settingValue == null) {
                m_settingGroup.getSettingValues().remove(getPath());
            } else {            
                m_settingGroup.getSettingValues().put(getPath(), m_settingValue);
            }
        }
    }

    public SettingValue getSettingValue() {
        return m_settingValue;
    }
    
    public void setSettingValue(SettingValue settingValue) {
        if (settingValue == null || settingValue.getValue() == null) {
            m_settingValue = null;                    
        } else {
            m_settingValue = settingValue;
        }
        updateSettingValues();
    }

    public SettingGroup getSettingGroup() {
        return m_settingGroup;
    }
    
    public void setSettingGroup(SettingGroup settingGroup) {
        m_settingGroup = settingGroup;
        updateSettingValues();
    }
    
    public String getPath() {
        String path;
        if (m_settingGroup == null) {
            path = "";
        } else {
            path = m_settingGroup.getPath() + "/" + getName();
        }

        return path;
    }
    
    protected Object clone() {
        Setting clone;
        try {
            clone = (Setting) super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException("clone SettingMeta is a required operations", e);
        }
        return clone;
    }

    /**
     * @return null always
     */
    public Setting getSetting(int index_) {
        return null;
    }

    /**
     * @throws IllegalArgumentException  Cannot put settings into another setting, only groups
     */
    public Setting addSetting(Setting setting_) {
        throw new IllegalArgumentException("Cannot put settings into another setting, only groups");
    }

    /**
     * @throws IllegalArgumentException  Cannot get settings from another setting, only groups
     */
    public Setting getSetting(String name_) {
        throw new IllegalArgumentException("Cannot get settings from another setting, only groups");
    }      
    
    public String getDefaultValue() {
        return m_defaultValue;
    }

    public void setDefaultValue(String defaultValue) {
        m_defaultValue = defaultValue;
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getValue() {
        return m_settingValue == null ? null : m_settingValue.getValue();
    }

    public void setValue(String value) {
        if (value != null) {
            setSettingValue(new SettingValue(getPath(), value));        
        } else {
            setSettingValue(null);
        }
    }

    public String getType() {
        return m_type;
    }

    public void setType(String type) {
        m_type = type;
    }
    
    public void addPossibleValue(String value) {
        if (m_possibleValues == null) {
            m_possibleValues = new ArrayList();
        }
        m_possibleValues.add(value);
    }

    public List getPossibleValues() {
        return m_possibleValues;
    }

    public void setPossibleValues(List possibleValues) {
        m_possibleValues = possibleValues;
    }

    /*
     * M A P  I M P L E M E N T A T I O N
     */

    public int size() {
        return 0;
    }

    public void clear() {
    }

    public boolean isEmpty() {
        return true;
    }

    public boolean containsKey(Object key_) {
        return false;
    }

    public boolean containsValue(Object value_) {
        return false;
    }

    public Collection values() {
        return Collections.EMPTY_LIST;
    }

    public void putAll(Map t_) {
    }

    public Set entrySet() {
        return Collections.EMPTY_SET;
    }

    public Set keySet() {
        return Collections.EMPTY_SET;
    }

    public Object get(Object key_) {
        return null;
    }

    public Object remove(Object key_) {
        return null;
    }

    public Object put(Object key_, Object value_) {
        return null;
    }

    /*
     *  E N D  M A P I M P L E M E N T A T I O N
     */
}
