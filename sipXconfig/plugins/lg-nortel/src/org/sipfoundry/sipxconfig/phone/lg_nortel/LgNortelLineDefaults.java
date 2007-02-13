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
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class LgNortelLineDefaults {

    private LineInfo m_lineInfo;

    public LgNortelLineDefaults(LineInfo lineInfo) {
        m_lineInfo = lineInfo;
    }

    @SettingEntry(path = "VOIP/proxy_address")
    public String getProxyAddress() {
        return m_lineInfo.getRegistrationServer();
    }

    @SettingEntry(path = "VOIP/proxy_port")
    public String getProxyPort() {
        return m_lineInfo.getRegistrationServerPort();
    }

    @SettingEntry(path = "VOIP/name")
    public String getUserName() {
        return m_lineInfo.getUserId();
    }

    @SettingEntry(path = "VOIP/displayname")
    public String getDisplayname() {
        return m_lineInfo.getDisplayName();
    }

    @SettingEntry(path = "VOIP/authname")
    public String getAuthName() {
        return m_lineInfo.getUserId();
    }

    @SettingEntry(path = "VOIP/password")
    public String getPassword() {
        return m_lineInfo.getPassword();
    }

    public static LineInfo getLineInfo(DeviceDefaults defaults, Line line) {
        User user = line.getUser();
        LineInfo lineInfo = new LineInfo();
        if (user != null) {
            lineInfo.setDisplayName(user.getDisplayName());
            lineInfo.setPassword(user.getSipPassword());
            lineInfo.setUserId(user.getUserName());
            lineInfo.setExtension(user.getExtension(true));
        }
        lineInfo.setRegistrationServer(defaults.getDomainName());
        lineInfo.setRegistrationServerPort(defaults.getProxyServerSipPort());
        return lineInfo;
    }

    public static void setLineInfo(LgNortelPhone phone, Line line, LineInfo lineInfo) {
        // TODO Auto-generated method stub
    }
}
