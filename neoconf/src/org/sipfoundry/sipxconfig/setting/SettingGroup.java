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

import org.apache.commons.collections.map.LinkedMap;

/**
 * Meta information about a group of settings, can contain nested
 * SettingModels.  Order is preserved
 */
public class SettingGroup extends SettingImpl implements Cloneable {
    
    private LinkedMap m_children = new LinkedMap();
        
    /** 
     * Root setting group and bean access only
     */
    public SettingGroup() {
    }
        
    public SettingGroup(String name) {
        super(name);
    }
    
    /**
     * includes deep copy of all childen 
     */
    public Setting copy() {
        SettingGroup copy = (SettingGroup) super.copy();
        copy.m_children = new LinkedMap();
        for (int i = 0; i < m_children.size(); i++) {
            Setting child = (Setting) m_children.getValue(i);            
            copy.addSetting(child.copy());
        }

        return copy;
    }

    public void acceptVisitor(SettingVisitor visitor) {
        visitor.visitSettingGroup(this);
    }

    /**
     * adds the setting to this group collection along with setting the
     * group on the setting 
     */
    public Setting addSetting(Setting setting) {
        setting.setSettingGroup(this);
        m_children.put(setting.getName(), setting);
        
        return setting;
    }

    public Setting getSetting(String name) {
        return (Setting) m_children.get(name);
    }

    public Collection getValues() {
        return m_children.values();
    }
}
