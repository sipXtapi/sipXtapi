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
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Grandstream business functions for line meta setting
 */
public class GrandstreamBtLine extends GrandstreamLine {

    public static final String FACTORY_ID = "grandstreamBtLine";

    public static final String PORT = "port";

    public static final String DISPLAY_NAME = "P3";

    public static final String UID = "P35";

    public static final String LOGIN_ID = "P36";

    public static final String PASSWORD = "P34";

    public static final String FIFTYSIXTY = "5060";

    /** while building model set root so getters/setting operation on this setting set*/
    private Setting m_root;

    public GrandstreamBtLine() {
        setModelFile("grandstream/line.xml");
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
        return getPhone().getSettings().getSetting(GrandstreamPhone.SIP).getSetting("P48").getValue();
    }

    public String getRegistrationServerPort() {
        return FIFTYSIXTY;
    }

    private Setting getRoot() {
        return m_root != null ? m_root : getSettings();
    }

    private Setting getLineSet() {
        return getRoot().getSetting(PORT);
    }

    public String getUserId() {
        return getLineSet().getSetting(UID).getValue();
    }

    public void setUserId(String userId) {
        getLineSet().getSetting(UID).setValue(userId);
    }

    public String getLoginId() {
        return getLineSet().getSetting(LOGIN_ID).getValue();
    }

    public void setLoginId(String userId) {
        getLineSet().getSetting(LOGIN_ID).setValue(userId);
    }

    public String getPwd() {
        return getLineSet().getSetting(PASSWORD).getValue();
    }

    public void setPwd(String pwd) {
        getLineSet().getSetting(PASSWORD).setValue(pwd);
    }

    public String getDisplayName() {
        return getLineSet().getSetting(DISPLAY_NAME).getValue();
    }

    public void setDisplayName(String userId) {
        getLineSet().getSetting(DISPLAY_NAME).setValue(userId);
    }
}
