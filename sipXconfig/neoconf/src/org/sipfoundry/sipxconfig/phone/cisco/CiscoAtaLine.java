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
package org.sipfoundry.sipxconfig.phone.cisco;

import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * CiscoAta business functions for line meta setting
 */
public class CiscoAtaLine extends CiscoLine {

    public static final String FACTORY_ID = "ciscoAtaLine";

    public CiscoAtaLine() {
        setModelFile("cisco/ata-line.xml");
    }

    public Object getAdapter(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "port/LoginID");
            adapter.addMapping(LineSettings.USER_ID, "port/UID");            
            adapter.addMapping(LineSettings.PASSWORD, "port/PWD");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "port/DisplayName");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "sip/Proxy");
            // sip/SIPPort for outbound proxy?
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }
}
