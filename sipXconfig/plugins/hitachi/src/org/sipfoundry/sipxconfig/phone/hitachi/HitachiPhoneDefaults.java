/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class HitachiPhoneDefaults {

    private DeviceDefaults m_defaults;

    public HitachiPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    @SettingEntry(paths = { "SERVER_SETTINGS/1st_Proxy", "SERVER_SETTINGS/1st_Registrar" })
    String getDomainName() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "SERVER_SETTINGS/Domain_Realm")
    String getAuthorizationRealm() {
        return m_defaults.getAuthorizationRealm();
    }
}
