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
package org.sipfoundry.sipxconfig.phone;

import java.util.Map;


/**
 * Base class for vendor specific setting management
 */
abstract public class AbstractSettings {

    private SettingSet m_settings;
    
    public AbstractSettings(SettingSet settings) {
        m_settings = settings;
    }
    
    public Map getEditableSettings() {        
        return m_settings.getSettings();
    }
    
    /**
     * Test whether there already exists a settting by that name
     * 
     * @param name
     * @return
     */
    public boolean hasSetting(String name) {
        return m_settings.getSettings().containsKey(name);
    }
    
    public SettingSet getSettings(String name) {
        return (SettingSet) getSettings().getSetting(name);
    }
    
    public Setting getSetting(String name) {
        return getSettings().getSetting(name);
    }
    
    /**
     * Will add a new setting if one doesn't exists 
     * 
     * @param name
     * @param value
     */
    public SettingSet defaultSettings(String name) {
        SettingSet settings = getSettings(name);
        if (settings == null) {
            m_settings.addSetting(settings = new SettingSet(name));
        }

    	return settings;
    }
    
    /**
     * Will add a new setting if one doesn't exists 
     * 
     * @param name
     * @param value
     */
    public Setting defaultSetting(String name, Object value) {
        Setting setting = getSetting(name);
        if (setting == null) {
            m_settings.addSetting(setting = new Setting(name, value));
        }
        
        return setting;
    }
      
    public SettingSet getSettings() {
        return m_settings;
    }
}
