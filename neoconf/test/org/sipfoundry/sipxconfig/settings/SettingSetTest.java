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

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Comments
 */
public class SettingSetTest extends TestCase {
    
    public void testIterator() {
        SettingSet root = new SettingSet();
        SettingSet si = new SettingSet();
        root.addSetting(si);
        for (int i = 0; i < 10; i++) {
            si.addSetting(new Setting("setting-" + i, new Integer(i)));
            SettingSet sj = new SettingSet("settings-" + i);
            si.addSetting(sj);
            for (int j = 0; j < 10; j++) {
                sj.addSetting(new Setting("settings-" + i + "-" + j, new Integer(i * j)));                
            }
        }
    }
    
    public void testLoadSave() {
        MockControl settingsControl = MockControl.createControl(SettingsDao.class);
        SettingsDao settingsDao = (SettingsDao) settingsControl.getMock();
        SettingSet rootSettings = new SettingSet("root");
        SettingSet networkSettings = new SettingSet(SettingSet.NETWORK_SETTINGS);
        networkSettings.addSetting(new Setting("tftpServer"));
        rootSettings.addSetting(networkSettings);        
        Endpoint endpoint = new Endpoint();
        settingsControl.expectAndReturn(settingsDao.loadSettings(endpoint), rootSettings);
        settingsDao.saveSettings(rootSettings);
        settingsControl.replay();
        
        SettingSet settings = settingsDao.loadSettings(endpoint);
        SettingSet network = (SettingSet) settings.getSetting(SettingSet.NETWORK_SETTINGS);
        assertNotNull(network);
        network.setSetting("tftpServer", "localhost");
        settingsDao.saveSettings(settings);
        
        settingsControl.verify();        
    }
}
