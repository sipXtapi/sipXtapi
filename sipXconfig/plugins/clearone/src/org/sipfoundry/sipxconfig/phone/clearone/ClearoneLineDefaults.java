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
    private Line m_line;
    private DeviceDefaults m_defaults;

    public ClearoneLineDefaults(Line line, DeviceDefaults defaults) {
        m_line = line;
        m_defaults = defaults;
    }

    @SettingEntry(path = "sip/sip_username")
    public String getUserName() {
        m_line.getDisplayLabel();
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getUserName();
    }

    @SettingEntry(path = "sip/sip_password")
    public String getSipPassword() {
        User user = m_line.getUser();
        if (user == null) {
            return null;
        }
        return user.getSipPassword();
    }

    @SettingEntry(path = "sip/outbound_sip_proxy")
    public String getSipOutbandProxy() {
        return m_defaults.getProxyServerAddr();

    }

    @SettingEntry(path = "sip/sip_proxy_server")
    public String getSipProxyServer() {
        return m_defaults.getDomainName();

    }

    @SettingEntry(paths = { "sip/sip_proxy_port", "sip/outbound_proxy_port" })
    public String getSipProxyPort() {
        return m_defaults.getProxyServerSipPort();
    }
}
