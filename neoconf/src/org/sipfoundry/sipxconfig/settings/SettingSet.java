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
package org.sipfoundry.sipxconfig.settings;

import java.util.Map;
import java.util.TreeMap;

/**
 * Set of data for a configurable object
 */
public class SettingSet extends Setting {
    
    /** settings regarding phone's network */
    public static final String NETWORK_SETTINGS = "network";

    private Map m_settings;

    public SettingSet() {
        this(null);
    }
    
    public SettingSet(String name) {
        setName(name);
        setValue(this);
        m_settings = new TreeMap();
    }
    
    public void addSetting(Setting setting) {
        setting.setParent(this);
        m_settings.put(setting.getName(), setting);
    }
    
    public Setting getSetting(String settingName) {
        return (Setting) m_settings.get(settingName);
    }
    
    public void setSetting(String settingName, Object value) {
        Setting setting = getSetting(settingName);
        if (setting == null) {
            throw new IllegalArgumentException("setting does not exist " + settingName);
        }
        setting.setValue(value);
    }
    
    /**
     * override to blank out stored value. it's not used
     */
    public String getString() {
        return null;
    }

    public Map getSettings() {
        return m_settings;
    }
    
    public void setSettings(Map settings) {
        // copy into new map, to keep order
        m_settings = new TreeMap();
        m_settings.putAll(settings);
    }
}
