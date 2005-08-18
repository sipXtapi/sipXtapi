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

import java.io.File;
import java.io.IOException;

import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class SipxServer implements Server {
    private String m_configDirectory;

    private ConfigFileStorage m_storage;

    public Setting getSettings() {
        Setting settingModel = getSettingModel();
        m_storage.decorate(settingModel);        
        return settingModel;
    }

    private Setting getSettingModel() {
        File settingDir = new File(m_configDirectory, "commserver");
        File modelDefsFile = new File(settingDir, "server.xml");
        Setting model = new XmlModelBuilder(m_configDirectory).buildModel(modelDefsFile).copy();

        return model;
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
}
