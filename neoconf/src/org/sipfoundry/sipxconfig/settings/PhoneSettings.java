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

import org.sipfoundry.sipxconfig.phone.AbstractSettings;
import org.sipfoundry.sipxconfig.phone.SettingSet;

/**
 * Comments
 */
public class PhoneSettings extends AbstractSettings {
    
    public final static String NETWORK_SETTINGS = "networkSettings";
    
    private NetworkSettings m_network;

    public PhoneSettings(SettingSet settings) {
        super(settings);
        setNetworkSettings(new NetworkSettings(defaultSettings(NETWORK_SETTINGS)));
    }
    
    public NetworkSettings getNetworkSettings() {
        return m_network;
    }
    
    public void setNetworkSettings(NetworkSettings network) {
        m_network = network;
    }
}
