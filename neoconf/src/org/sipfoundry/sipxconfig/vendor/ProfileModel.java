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
package org.sipfoundry.sipxconfig.vendor;

import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;


public class ProfileModel {
    
    private PolycomPhone m_phone;
    
    private Endpoint m_endpoint;
    
    public ProfileModel(PolycomPhone phone, Endpoint endpoint) {
        m_phone = phone;
        m_endpoint = endpoint;
    }

    public Collection getRegistrations() {
        ArrayList registrations = new ArrayList(m_phone.getMaxLineCount());

        SettingGroup root = m_endpoint.getSettings(m_phone);
        SettingGroup reg = (SettingGroup) root.getSetting(PolycomPhone.REGISTRATION_SETTINGS);
        registrations.add(reg.getValues());

        // copy in blank registrations of all unused lines
        for (int i = 1; i < m_phone.getMaxLineCount(); i++) {
            SettingGroup regCopy = (SettingGroup) reg.getCopy(new ValueStorage());
            registrations.add(regCopy.getValues());
        }
        
        return registrations;
    }
}
