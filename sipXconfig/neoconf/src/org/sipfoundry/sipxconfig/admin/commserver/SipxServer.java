/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.IOException;

import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class SipxServer implements Server {
    private String m_configDirectory;

    private ConfigFileStorage m_storage;
    
    private Setting m_settingModel;
    
    /**
     * @return NOTE: returns new settings instance each call 
     */
    public Setting getSettings() {
        Setting settings = getSettingModel().copy();
        m_storage.decorate(settings);
        
        return settings;
    }

    public Setting getSettingModel() {
        return m_settingModel;
    }
    
    public void setSettingModel(Setting model) {
        m_settingModel = model;
    }

    public void applySettings() {
        try {
            m_storage.flush();
        } catch (IOException e) {
            // TODO: catch and report as User Exception
            throw new RuntimeException(e);
        }
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
    }
    
    public ServerSettings getServerSettings() {
        SettingBeanAdapter adapter = new SettingBeanAdapter(ServerSettings.class);
        adapter.setSetting(getSettings());
        adapter.addMapping("domainName", "domain/SIPXCHANGE_DOMAIN_NAME");
        return (ServerSettings) adapter.getImplementation();        
    }
}
