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
import java.util.Iterator;
import java.util.List;

import org.apache.commons.beanutils.BeanUtils;

/**
 * Base class for all items describing and using setting.  
 */    
public class Setting implements Cloneable, AvoidEclipseWarningHack {

    public static final String NULL_VALUE = new String();

    private String m_label;

    private String m_type;

    private String m_name;
    
    private String m_description;
    
    private String m_defaultValue;

    private List m_possibleValues;
    
    private String m_profileName;
    
    private SettingGroup m_settingGroup;
    
    private ValueStorage m_valueStorage;    
    
    /**
     * bean access only, must set name before valid object
     */
    public Setting() {
    }
    
    public Setting(String name) {
        setName(name);
    }

    public Setting getCopy(ValueStorage valueStorage) {
        Setting clone = (Setting) clone();
        clone.setValueStorage(valueStorage);
        
        return clone;
    }
        
    void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }
    
    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }

    public SettingGroup getSettingGroup() {
        return m_settingGroup;
    }
    
    public void setSettingGroup(SettingGroup settingGroup) {
        m_settingGroup = settingGroup;
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

    /**
     * @return label if set, otherwise return name as label.
     */
    public String getLabel() {
        return m_label != null ? m_label : m_name;
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
    
    public String getProfileName() {
        return m_profileName == null ? m_name : m_profileName;
    }
    
    public void setProfileName(String profileName) {
        m_profileName = profileName;
    }

    public String getProfileValue() {
        String value = getValue();
        return value == null ? m_defaultValue : value;
    }

    public String getValue() {
        
        // technically not trying to mutate, but
        // probably not a valid state. could relax if valid
        // case comes up.
        checkImmutable();
        
        String value = (String) m_valueStorage.get(getPath());
        
        return value == null ? m_defaultValue : value;
    }

    public void setValue(String value) {
        checkImmutable();
        String defValue = getDefaultValue();
        if (value == null) {
            if (defValue == null) {
                m_valueStorage.remove(getPath());
            } else {
                m_valueStorage.put(getPath(), NULL_VALUE);
            }
        } else {
            if (value.equals(defValue)) {
                m_valueStorage.remove(getPath());
            } else {
                m_valueStorage.put(getPath(), value);                
            }
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

    protected void checkImmutable() {
        if (m_valueStorage == null) {
            throw new UnsupportedOperationException("Immutable copy, you must " 
                    + "call getCopy on root SettingGroup instance");
        }        
    }
    
    public String dump() {
        StringBuffer sb = new StringBuffer();
        sb.append(getPath()).append(": ");
        try {
            sb.append(BeanUtils.describe(this).toString());
        } catch (Exception e) {            
            e.printStackTrace();
            sb.append(e.toString());
        }
        
        return sb.toString();
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }
    
    public Collection getValues() {
        return Collections.EMPTY_LIST;
    }
    
    public Collection list(SettingFilter filter) {
        Collection bucket = new ArrayList();
        list(filter, this, bucket);
        
        return bucket;
    }
    
    public void list(SettingFilter filter, Setting root, Collection bucket) {
        Iterator children = getValues().iterator();
        while (children.hasNext()) {
            Setting setting = (Setting) children.next();
            if (filter.acceptSetting(root, setting)) {
                bucket.add(setting);
            }
            setting.list(filter, root, bucket);
        }
    }
}

/**
 * Part of composite design pattern with SettingGroup where subclass implements
 * functions defined but not used in base class
 */
interface AvoidEclipseWarningHack {
    Setting getSetting(int i);
    Setting addSetting(Setting s);
    Setting getSetting(String s);
}

