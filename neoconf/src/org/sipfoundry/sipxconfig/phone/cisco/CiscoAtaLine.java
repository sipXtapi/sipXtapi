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
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * CiscoAta business functions for line meta setting
 */
public class CiscoAtaLine extends CiscoLine {

    public static final String FACTORY_ID = "ciscoAtaLine";

    public static final String DISPLAY_NAME = "DisplayName";

    public static final String UID = "UID";

    public static final String LOGIN_ID = "LoginID";

    public static final String PASSWORD = "PWD";

    /** while building model set root so getters/setting operation on this setting set*/
    private Setting m_root;
    
    public CiscoAtaLine() {
        setModelFile("cisco/ata-line.xml");
    }

    public void setDefaults(Setting settings) {

        // HACK : temporarily set root setting to trick utility methods to
        // operate on this setting set.
        m_root = settings;
        try {
            User u = getLineData().getUser();
            if (u != null) {
                setUserId(u.getDisplayId());
                setLoginId(u.getDisplayId());
                String dispname = u.getDisplayName();
                if (dispname != null) {
                    setDisplayName(dispname);
                }

                String password = getPhoneContext().getClearTextPassword(u);
                setPwd(password);
            }
        } finally {
            m_root = null;
        }
    }

    public String getRegistrationServerAddress() {
        return getPhone().getSettings().getSetting(CiscoPhone.SIP).getSetting("Proxy").getValue();
    }

    public String getRegistrationServerPort() {
        return getPhone().getSettings().getSetting(CiscoPhone.SIP).getSetting("SIPPort").getValue();
    }

    private Setting getRoot() {
        return m_root != null ? m_root : getSettings();
    }

    private Setting getPort() {
        return getRoot().getSetting(CiscoPhone.PORT);
    }

    public String getUserId() {
        return getPort().getSetting(UID).getValue();
    }

    public void setUserId(String userId) {
        getPort().getSetting(UID).setValue(userId);
    }

    public String getLoginId() {
        return getPort().getSetting(LOGIN_ID).getValue();
    }

    public void setLoginId(String userId) {
        getPort().getSetting(LOGIN_ID).setValue(userId);
    }

    public String getPwd() {
        return getPort().getSetting(PASSWORD).getValue();
    }

    public void setPwd(String pwd) {
        getPort().getSetting(PASSWORD).setValue(pwd);
    }

    public String getDisplayName() {
        return getPort().getSetting(DISPLAY_NAME).getValue();
    }

    public void setDisplayName(String userId) {
        getPort().getSetting(DISPLAY_NAME).setValue(userId);
    }
}
