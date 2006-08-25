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

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;

/**
 * Meta information about a group of settings, can contain nested SettingModels. Order is
 * preserved
 */
public class SettingSet extends AbstractSetting implements Cloneable, Serializable {

    private LinkedHashMap<String, Setting> m_children = new LinkedHashMap<String, Setting>();

    /**
     * Root setting group and bean access only
     */
    public SettingSet() {
    }

    public SettingSet(String name) {
        super(name);
    }

    /**
     * includes deep copy of all childen
     */
    public Setting copy() {
        SettingSet copy = (SettingSet) shallowCopy();
        for (Setting child : m_children.values()) {
            copy.addSetting(child.copy());
        }
        return copy;
    }

    protected Setting shallowCopy() {
        SettingSet copy = (SettingSet) super.copy();
        copy.m_children = new LinkedHashMap<String, Setting>();
        return copy;
    }

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSettingGroup(this);
        for (Setting setting : m_children.values()) {
            setting.acceptVisitor(visitor);
        }
    }

    /**
     * adds the setting to this group collection along with setting the group on the setting
     */
    public Setting addSetting(Setting setting) {
        setting.setParent(this);

        Setting existingChild = m_children.put(setting.getName(), setting);
        if (existingChild != null) {
            Collection<Setting> grandChildren = existingChild.getValues();
            for (Setting grandChild : grandChildren) {
                setting.addSetting(grandChild);
            }
        }

        return setting;
    }

    public Setting getSetting(String name) {
        return SettingUtil.getSettingByPath(m_children, this, name);
    }

    public Collection<Setting> getValues() {
        return m_children.values();
    }

    /**
     * @param requiredType returned Setting must be
     * @return default setting in the set - usually first child
     */
    public Setting getDefaultSetting(Class requiredType) {
        for (Iterator i = getValues().iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            if (requiredType.isAssignableFrom(setting.getClass())) {
                return setting;
            }
        }
        return null;
    }

    public String getDefaultValue() {
        throw new UnsupportedOperationException();
    }

    public String getValue() {
        throw new UnsupportedOperationException();
    }

    public void setValue(String value) {
        throw new UnsupportedOperationException();
    }
}
