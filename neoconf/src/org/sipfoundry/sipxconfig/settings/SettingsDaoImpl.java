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

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class SettingsDaoImpl extends HibernateDaoSupport implements SettingsDao {

    public void saveSettings(SettingSet settingsTemp) {        
        throw new IllegalArgumentException("saveSettings not implemented yet");
    }
    
    public SettingSet loadSettings(Endpoint endpointTemp) {        
        throw new IllegalArgumentException("loadSettings not implemented yet");
    }
}
