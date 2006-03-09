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

import org.sipfoundry.sipxconfig.setting.type.SettingType;

/**
 * Wrap another setting object, delegate ALL methods to the Settting class
 */
public abstract class SettingDecorator implements Setting, Cloneable {

    private Setting m_delegate;

    /** BEAN ACCESS ONLY */
    public SettingDecorator() {
    }

    public SettingDecorator(Setting delegate) {
        m_delegate = delegate;
    }

    protected Setting getDelegate() {
        return m_delegate;
    }

    protected void setDelegate(Setting delegate) {
        m_delegate = delegate;
    }

    public String getParentPath() {
        return m_delegate.getParentPath();
    }

    public void setParentPath(String path) {
        m_delegate.setParentPath(path);
    }

    public String getPath() {
        return m_delegate.getPath();
    }

    public Setting addSetting(Setting setting) {
        return m_delegate.addSetting(setting);
    }

    public Setting getSetting(String name) {
        return m_delegate.getSetting(name);
    }

    public String getLabel() {
        return m_delegate.getLabel();
    }

    public String getName() {
        return m_delegate.getName();
    }

    public void setName(String name) {
        m_delegate.setName(name);
    }

    public String getProfileName() {
        return m_delegate.getProfileName();
    }

    public String getValue() {
        return m_delegate.getValue();
    }

    public Object getTypedValue() {
        return getType().convertToTypedValue(getValue());
    }
    
    public void setTypedValue(Object value) {
        setValue(getType().convertToStringValue(value));
    }

    public String getDefaultValue() {
        return m_delegate.getDefaultValue();
    }

    public void setValue(String value) {
        m_delegate.setValue(value);
    }

    public SettingType getType() {
        return m_delegate.getType();
    }

    public void setType(SettingType type) {
        m_delegate.setType(type);
    }

    public String getDescription() {
        return m_delegate.getDescription();
    }

    public Collection getValues() {
        return m_delegate.getValues();
    }

    public boolean isAdvanced() {
        return m_delegate.isAdvanced();
    }

    public boolean isHidden() {
        return m_delegate.isHidden();
    }

    /**
     * Does not use delegate! Assumes subclass is a Setting and not a SettingGroup
     */
    public void acceptVisitor(SettingVisitor visitor) {

        // Does not use delegate! because "visitSetting(this)" would pass in
        // the delegate and not this decorator. So we have to mimic what setting
        // group and setting value do
        // m_delegate.acceptVisitor(visitor);
        //
        if (getValues().size() == 0) {
            visitor.visitSetting(this);
        } else {
            visitor.visitSettingGroup(this);
            Iterator children = getValues().iterator();
            while (children.hasNext()) {
                Setting setting = (Setting) children.next();
                setting.acceptVisitor(visitor);
            }
        }
    }

    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException("Cannot clone setting decorator", e);
        }
    }

    public Setting copy() {
        SettingDecorator copy = (SettingDecorator) clone();
        copy.m_delegate = m_delegate.copy();
        return copy;
    }
}
