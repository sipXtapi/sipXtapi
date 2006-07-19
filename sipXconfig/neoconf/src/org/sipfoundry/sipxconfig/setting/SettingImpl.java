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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;

public class SettingImpl implements Setting, Cloneable, NamedObject {
    private static final SettingValue2 NULL = new SettingValueImpl(null);
    private String m_label;
    private SettingType m_type = StringSetting.DEFAULT;
    private String m_name = StringUtils.EMPTY;
    private String m_description;
    private SettingValue2 m_originalProfileName;
    private Setting m_parent;
    private SettingValue2 m_originalValue = NULL;
    private boolean m_advanced;
    private boolean m_hidden;
    private SettingModel2 m_model;

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

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSetting(this);
    }

    /**
     * @throws IllegalArgumentException Cannot put settings into another setting, only groups
     */
    public Setting addSetting(Setting setting_) {
        throw new IllegalArgumentException(
                "Cannot put settings into another setting, only groups");
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
        SettingValue2 value = m_originalProfileName;
        if (value == null) {
            value = new SettingValueImpl(getName());
        }
        if (m_model != null) {
            value = m_model.getProfileName(this, value);
        }

        return value.getValue();
    }

    public void setProfileName(String profileName) {
        m_originalProfileName = new SettingValueImpl(profileName);
    }

    public String getValue() {
        SettingValue2 value = m_originalValue;
        if (m_model != null) {
            value = m_model.getSettingValue(this, m_originalValue);
        }

        return value.getValue();
    }

    public Object getTypedValue() {
        return getType().convertToTypedValue(getValue());
    }

    public void setValue(String value) {
        if (m_model == null) {
            m_originalValue = new SettingValueImpl(value);
        } else {
            m_model.setSettingValue(this, value);
        }
    }

    public void setTypedValue(Object value) {
        setValue(getType().convertToStringValue(value));
    }

    public String getDefaultValue() {
        SettingValue2 value = m_originalValue;
        if (m_model != null) {
            value = m_model.getDefaultSettingValue(this, m_originalValue);
        }

        return value.getValue();
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

    public void setModel(SettingModel2 model) {
        m_model = model;
    }

    public SettingModel2 getModel() {
        return m_model;
    }
}
