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
public class SettingGroup extends SettingImpl {
    
    private LinkedMap m_delegate = new LinkedMap();
        
    /** 
     * Root setting group and bean access only
     */
    public SettingGroup() {
    }
        
    public SettingGroup(String name) {
        super(name);
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
        m_delegate.put(setting.getName(), setting);
        
        return setting;
    }

    public Setting getSetting(String name) {
        return (Setting) m_delegate.get(name);
    }

    public Collection getValues() {
        return m_delegate.values();
    }
}
