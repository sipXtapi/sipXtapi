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
package org.sipfoundry.sipxconfig.phone.clearone;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class ClearoneLineDefaults {
    static final String REGISTRATION_SERVER_SETTING = "sip/sip_proxy_server";
    static final String PASSWORD_SETTING = "sip/sip_password";
    static final String USER_ID_SETTING = "sip/sip_username";
    static final String DISPLAY_NAME_SETTING = "sip/sip_displayname";
    
    private Line m_line;
    private DeviceDefaults m_defaults;

    public ClearoneLineDefaults(Line line, DeviceDefaults defaults) {
        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(path = DISPLAY_NAME_SETTING)
    public String getDisplayName() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getDisplayName();
    }

    @SettingEntry(path = USER_ID_SETTING)
    public String getUserName() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getUserName();
    }

    @SettingEntry(path = PASSWORD_SETTING)
    public String getSipPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getSipPassword();
    }

    @SettingEntry(path = "sip/outbound_sip_proxy")
    public String getSipOutboundProxy() {
        return m_defaults.getProxyServerAddr();

    }

    @SettingEntry(path = REGISTRATION_SERVER_SETTING)
    public String getSipProxyServer() {
        return m_defaults.getDomainName();

    }

    @SettingEntry(paths = { "sip/sip_proxy_port", "sip/outbound_proxy_port" })
    public String getSipProxyPort() {
        return m_defaults.getProxyServerSipPort();
    }
}
