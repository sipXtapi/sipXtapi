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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;


public class PhoneConfiguration extends ConfigurationTemplate {
    
    public PhoneConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }

    public void addContext(VelocityContext context) {
        context.put("phone", this);
    }

    public Collection getRegistrations() {
        PolycomPhone phone = getPhone();
        ArrayList registrations = new ArrayList(phone.getMaxLineCount());

        SettingGroup root = getEndpoint().getSettings(phone);
        SettingGroup reg = (SettingGroup) root.getSetting(PolycomPhone.REGISTRATION_SETTINGS);
        registrations.add(reg.getValues());

        // copy in blank registrations of all unused lines
        for (int i = 1; i < phone.getMaxLineCount(); i++) {
            SettingGroup regCopy = (SettingGroup) reg.getCopy(new ValueStorage());
            registrations.add(regCopy.getValues());
        }
        
        return registrations;
    }
}
