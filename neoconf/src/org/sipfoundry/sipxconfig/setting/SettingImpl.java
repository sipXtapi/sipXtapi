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


public class SettingImpl implements Setting, Cloneable {

    private String m_label;

    private String m_type;

    private String m_name;
    
    private String m_description;

    private List m_possibleValues;
    
    private String m_profileName;
    
    private SettingGroup m_settingGroup;
    
    private String m_value;
    
    private boolean m_hidden;
    
    /**
     * bean access only, must set name before valid object
     */
    public SettingImpl() {
    }
    
    public SettingImpl(String name) {
        setName(name);
    }
    
    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException("Cannot clone setting", e);
        }
    }
    
    public Setting copy() {
        return (Setting) clone();
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
    
    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSetting(this);
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

    public String getValue() {        
        return m_value;
    }

    public void setValue(String value) {
        m_value = value;
    }
    
    /**
     * No wrapper, default value is what the real value is
     */
    public String getDefaultValue() {
        return getValue();
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

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }
    
    public Collection getValues() {
        return Collections.EMPTY_LIST;
    }
    
    public boolean isHidden() {
        return m_hidden;
    }
    
    public void setHidden(boolean hidden) {
        m_hidden = hidden;
    }
}
