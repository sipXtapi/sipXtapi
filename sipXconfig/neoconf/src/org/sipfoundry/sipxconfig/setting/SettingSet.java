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
import java.util.LinkedHashMap;

/**
 * Meta information about a group of settings, can contain nested SettingModels. Order is
 * preserved
 */
public class SettingSet extends SettingImpl implements Cloneable {

    private LinkedHashMap m_children = new LinkedHashMap();

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
        SettingSet copy = (SettingSet) super.copy();
        copy.m_children = new LinkedHashMap();
        Iterator i = m_children.values().iterator();
        while (i.hasNext()) {
            Setting child = (Setting) i.next();
            copy.addSetting(child.copy());
        }

        return copy;
    }

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSettingGroup(this);
        for (Iterator i = m_children.values().iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            setting.acceptVisitor(visitor);
        }
    }

    /**
     * adds the setting to this group collection along with setting the group on the setting
     */
    public Setting addSetting(Setting setting) {
        setting.setParentPath(getPath());

        Setting existingChild = (Setting) m_children.get(setting.getName());
        m_children.put(setting.getName(), setting);
        if (existingChild != null) {
            Collection grandChildren = existingChild.getValues();
            if (!grandChildren.isEmpty()) {
                Iterator igrandChildren = grandChildren.iterator();
                while (igrandChildren.hasNext()) {
                    Setting grandChild = (Setting) igrandChildren.next();
                    setting.addSetting(grandChild);
                }
            }
        }

        return setting;
    }

    public Setting getSetting(String name) {
        return SettingUtil.getSettingByPath(m_children, this, name);
    }

    public Collection getValues() {
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
}
