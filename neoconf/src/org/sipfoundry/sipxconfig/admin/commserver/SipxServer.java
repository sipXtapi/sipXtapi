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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class SipxServer implements Server {
    private String m_configDirectory;

    private ConfigFileStorage m_storage;
    
    private Setting m_settingModel;
    
    private Setting m_settings;
    
    private String m_domainName;
    
    private SipxReplicationContext m_sipxReplicationContext;
    
    private PhoneDefaults m_phoneDefaults;
    
    private CoreContext m_coreContext;
    
    /**
     * @return NOTE: returns new settings instance each call 
     */
    public Setting getSettings() {
        if (m_settings == null) {
            m_settings = getSettingModel();
        
            m_storage.decorate(m_settings);
        
            // XCF-552 - Domain name on a vanilla installation is `hostname -d`
            // evaluating shell expression is beyond the scope of the UI. Therefore we
            // inject the domain name set on this bean because it's been resolved already.
            // When we support the separation of commserver from general sipxconfig configuration,
            // I suspect domain name will be stored in a separate object and that drives the
            // modification of config.defs and is referenced directly by phoneContext and coreContext
            m_settings.getSetting(ServerSettings.DOMAIN_NAME).setValue(m_domainName);
        }

        return m_settings;
    }
    
    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }

    public Setting getSettingModel() {
        return m_settingModel;
    }
    
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }
    
    public void setPhoneDefaults(PhoneDefaults phoneDefaults) {
        m_phoneDefaults = phoneDefaults;
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void setSettingModel(Setting model) {
        m_settingModel = model;
    }

    public void applySettings() {
        try {
            handlePossibleDomainNameChange();
            m_storage.flush();
        } catch (IOException e) {
            // TODO: catch and report as User Exception
            throw new RuntimeException(e);
        }
    }
    
    void handlePossibleDomainNameChange() {
        String newDomainName = getServerSettings().getDomainName();        
        // bail if domain name wasn't changed.
        if (m_domainName.equals(newDomainName)) {
            return;
        }
        
        m_domainName = newDomainName;
        
        // unwelcome dependencies, resolve when domain name editing
        // refactored.
        m_phoneDefaults.setDomainName(m_domainName);
        m_coreContext.setDomainName(m_domainName);
        
        m_sipxReplicationContext.generateAll();        
    }
    

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
    }
    
    public ServerSettings getServerSettings() {
        SettingBeanAdapter adapter = new SettingBeanAdapter(ServerSettings.class);
        adapter.setSetting(getSettings());
        adapter.addMapping("domainName", ServerSettings.DOMAIN_NAME);
        return (ServerSettings) adapter.getImplementation();        
    }
}
