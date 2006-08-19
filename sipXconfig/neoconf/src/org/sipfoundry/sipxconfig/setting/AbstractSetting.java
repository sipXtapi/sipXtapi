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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public abstract class AbstractSetting implements Setting, NamedObject {
    private static final SettingValue2 NULL = new SettingValueImpl(null);

    private String m_label;
    private SettingType m_type = StringSetting.DEFAULT;
    private String m_name = StringUtils.EMPTY;
    private String m_description;
    private String m_profileName;
    private Setting m_parent;
    private boolean m_advanced;
    private boolean m_hidden;
    private SettingValue2 m_value = NULL;
    

    public AbstractSetting() {
        super();
    }

    public AbstractSetting(String name) {
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

    public Setting getParent() {
        return m_parent;
    }

    public void setParent(Setting parent) {
        m_parent = parent;
    }

    public String getPath() {
        Setting parent = getParent();
        if (parent == null) {
            return StringUtils.EMPTY;
        }

        if (parent.getParent() == null) {
            return getName();
        }

        return parent.getPath() + PATH_DELIM + getName();
    }

    public String getProfilePath() {
        Setting parent = getParent();
        if (parent == null) {
            return getProfileName();
        }
        return parent.getProfilePath() + PATH_DELIM + getProfileName();
    }

    /**
     * Correct Examples getSetting("a/b/c");
     * 
     * Incorrect Examples: getSetting("../a/b");
     * 
     * @throws IllegalArgumentException Cannot get settings from another setting, only groups
     */
    public Setting getSetting(String name) {
        return SettingUtil.getSettingByPath(null, this, name);
    }

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSetting(this);
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

    public void setProfileName(String profileName) {
        m_profileName = profileName;
    }

    public String getProfileName() {
        if (m_profileName == null) {
            return getName();
        }
        return m_profileName;
    }
    
    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
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

    /**
     * @throws IllegalArgumentException Cannot put settings into another setting, only groups
     */
    public Setting addSetting(Setting setting_) {
        throw new IllegalArgumentException(
                "Cannot put settings into another setting, only groups");
    }

    public SettingType getType() {
        return m_type;
    }

    public void setType(SettingType type) {
        m_type = type;
    }

    public void setTypedValue(Object value) {
        setValue(getType().convertToStringValue(value));
    }

    public Object getTypedValue() {
        return getType().convertToTypedValue(getValue());
    }
    
    public String getValue() {
        return m_value.getValue();
    }
    
    public void setValue(String value) {
        m_value = new SettingValueImpl(value);
    }
}
