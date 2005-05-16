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

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class SipxServer implements Server {
    private String m_configDirectory;
    
    private ValueStorage m_valueStorage = new ValueStorage();
    
    public Setting getSettings() {
        Setting settingModel = getSettingModel();
        return new SettingValue(m_valueStorage, settingModel);
    }
    
    private Setting getSettingModel() {
        File settingDir = new File(m_configDirectory, "commserver");
        File modelDefsFile = new File(settingDir, "server.xml");
        Setting model = new XmlModelBuilder().buildModel(modelDefsFile).copy();
        
        return model;
    }

    public void applySettings() {
        // TODO Auto-generated method stub        
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;        
    }    
}
