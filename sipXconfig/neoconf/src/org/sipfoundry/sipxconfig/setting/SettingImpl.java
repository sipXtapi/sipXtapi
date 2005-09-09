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
import java.util.Collections;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;


public class SettingImpl implements Setting, Cloneable, NamedObject {

    private String m_label;

    private SettingType m_type = StringSetting.DEFAULT;

    private String m_name = StringUtils.EMPTY;
    
    private String m_description;

    private String m_profileName;
    
    private String m_parentPath;
    
    private String m_value;
        
    private boolean m_advanced;

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
    
    public String getParentPath() {
        return m_parentPath;
    }
    
    /**
     * Resolve decorators
     */
    
    public void setParentPath(String path) {
        m_parentPath = path;
        Collection c = getValues();
        if (!c.isEmpty()) {
            Iterator i = c.iterator();
            while (i.hasNext()) {
                Setting child = (Setting) i.next();
                // recursive
                child.setParentPath(getPath());
            }
        }
    }
    
    public String getPath() {
        if (getParentPath() == null) {
            return getName();
        }
        
        return getParentPath() + PATH_DELIM + getName();
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
     * Correct Examples
     *   getSetting("a/b/c");
     *   
     * Incorrect Examples:
     *   getSetting("../a/b");
     * 
     * @throws IllegalArgumentException  Cannot get settings from another setting, only groups
     */
    public Setting getSetting(String name) {
        return SettingUtil.getSettingByPath(null, this, name);
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
    
    public Object getTypedValue() {
        return getType().convertToTypedValue(getValue());
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

    public SettingType getType() {
        return m_type;
    }

    public void setType(SettingType type) {
        m_type = type;
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
    
    public boolean isAdvanced() {
        return m_advanced;
    }
    
    public void setAdvanced(boolean advanced) {
        m_advanced = advanced;
    }
    
    public boolean isHidden() {
        return m_hidden;
    }
    
    public void setHidden(boolean hidden) {
        m_hidden = hidden;
    }
}
