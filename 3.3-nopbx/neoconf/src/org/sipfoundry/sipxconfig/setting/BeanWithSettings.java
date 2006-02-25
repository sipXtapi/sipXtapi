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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class BeanWithSettings extends BeanWithId {

    /**
     * While settings are getting decorated, this represents the settings that should be decorated
     */
    private Setting m_settings;

    private ValueStorage m_valueStorage;
       
    private Setting m_model;
    
    /** settings are lazy loaded */
    private boolean m_initializedSettings;

    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel() {
        return (m_model != null ? m_model.copy() : null);
    }
    
    public void setSettingModel(Setting model) {
        m_model = model;
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        if (!m_initializedSettings) {
            m_initializedSettings = true;
            setSettings(getSettingModel());
            defaultSettings();
            decorateSettings();
        }

        return m_settings;
    }
    
    protected void setSettings(Setting settings) {
        m_settings = settings;
    }
        
    protected void decorateSettings() {
        Setting settings = getSettings();
        if (settings == null) {
            return;
        }
        
        if (m_valueStorage == null) {
            m_valueStorage = new ValueStorage();
        }
        
        m_valueStorage.decorate(settings);
        setSettings(settings);
    }
    
    /**
     * Make adjustments to current settings based on object state sparing the user 
     * from having to specifiy them. Default implementation applys no defaults, override
     * to supply your own.
     */
    protected void defaultSettings() {
    }
    
    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }

    public String getSettingValue(String path) {
        return getSettings().getSetting(path).getValue();
    }

    public Object getSettingTypedValue(String path) {
        return getSettings().getSetting(path).getTypedValue();
    }

    public void setSettingValue(String path, String value) {
        getSettings().getSetting(path).setValue(value);
    }
}
