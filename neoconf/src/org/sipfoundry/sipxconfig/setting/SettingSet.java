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

import java.util.Map;
import java.util.TreeMap;



/**
 * Set of data for a configurable object
 */
public class SettingSet extends Setting {

    private static final long serialVersionUID = -2495309201272644336L;
    
    private Map m_settings;

    public SettingSet() {
        this(null); 
    }
    
    public SettingSet(String name) {
        super(name);
        setValue(this);
        m_settings = new TreeMap();
    }
    
    public Setting addSetting(Setting setting) {
        setting.setParent(this);
        m_settings.put(setting.getName(), setting);
        
        return setting;
    }
    
    public Setting getSetting(String settingName) {
        return (Setting) m_settings.get(settingName);
    }
    
    public void setSetting(String settingName, Object value) {
        Setting setting = getSetting(settingName);
        if (setting == null) {
            setting = new Setting(settingName);
            m_settings.put(settingName, setting);
        }
        setting.setValue(value);
    }
    
    public void setDefault(String settingName, Object def) {
        Setting setting = getSetting(settingName);        
        if (setting == null) {
            setting = new Setting(settingName);
            m_settings.put(settingName, setting);
        }
        setting.setDefault(def);        
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
        m_settings = settings;
    }
}
