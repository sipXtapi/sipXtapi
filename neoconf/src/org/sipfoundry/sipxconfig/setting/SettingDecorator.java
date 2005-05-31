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
    
    public Setting getParent() {
        return m_delegate.getParent();
    }

    public void setParent(Setting setting) {
        m_delegate.setParent(setting);        
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

    public void setLabel(String label) {
        m_delegate.setLabel(label);
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

    public void setProfileName(String profileName) {
        m_delegate.setProfileName(profileName);
    }

    public String getValue() {
        return m_delegate.getValue();
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

    public void setDescription(String description) {
        m_delegate.setDescription(description);
    }

    public Collection getValues() {
        return m_delegate.getValues();
    }

    public boolean isAdvanced() {
        return m_delegate.isAdvanced();
    }

    /** 
     * Does not use delegate! Assumes subclass is a Setting and not a SettingGroup
     */
    public void acceptVisitor(SettingVisitor visitor) {
        if (getValues().size() == 0) {
            visitor.visitSetting(this);
        } else {
            visitor.visitSettingGroup(this);
        }
    }

    public void setAdvanced(boolean hidden) {
        m_delegate.setAdvanced(hidden);
    }
    
    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException("Cannot clone setting decorator", e);
        }
    }
    
    public Setting copy() {
        return (Setting) clone();
    }
}
