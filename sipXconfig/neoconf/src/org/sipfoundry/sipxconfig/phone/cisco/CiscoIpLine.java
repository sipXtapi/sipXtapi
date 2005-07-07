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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.LineSettings;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

/**
 * Cisco business functions for line meta setting
 */
public class CiscoIpLine extends CiscoLine {

    public static final String FACTORY_ID = "ciscoIpLine";

    public CiscoIpLine() {
        setModelFile("cisco/ip-line.xml");
    }
    
    public Object getAdapter(Class interfac) {
        Object impl;
        if (interfac == LineSettings.class) {
            SettingBeanAdapter adapter = new SettingBeanAdapter(interfac);
            adapter.setSetting(getSettings());
            adapter.addMapping(LineSettings.AUTHORIZATION_ID, "line/authname");
            adapter.addMapping(LineSettings.USER_ID, "line/name");            
            adapter.addMapping(LineSettings.PASSWORD, "line/password");
            adapter.addMapping(LineSettings.DISPLAY_NAME, "line/displayname");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER, "proxy/address");
            adapter.addMapping(LineSettings.REGISTRATION_SERVER_PORT, "proxy/port");
            impl = adapter.getImplementation();
        } else {
            impl = super.getAdapter(interfac);
        }
        
        return impl;
    }

    public void setDefaults() {
        super.setDefaults();
        
        User u = getLineData().getUser();
        if (u != null) {
            getSettings().getSetting("line/shortname").setValue(u.getDisplayId());
        }
    }
}
