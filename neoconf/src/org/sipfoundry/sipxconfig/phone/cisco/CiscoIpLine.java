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
 * Cisco business functions for line meta setting
 */
public class CiscoIpLine extends CiscoLine {

    public static final String FACTORY_ID = "ciscoIpLine";

    public static final String PHONELINE = "line";

    public static final String PROXY = "proxy";

    public static final String DISPLAY_NAME = "displayname";

    public static final String UID = "name";

    public static final String LOGIN_ID = "authname";

    public static final String PASSWORD = "password";

    public static final String SHORTNAME = "shortname";

    public static final String ADDRESS = "address";

    /** while building model set root so getters/setting operation on this setting set*/
    private Setting m_root;

    public CiscoIpLine() {
        setModelFile("cisco/ip-line.xml");
    }
    
    public void setDefaults(Setting settings) {

        // HACK : temporarily set root setting to trick utility methods to
        // operate on this setting set.
        m_root = settings;
        try {
            User u = getLineData().getUser();
            if (u != null) {
                setShortName(u.getDisplayId());
                setUserId(u.getDisplayId());
                setLoginId(u.getDisplayId());
                String dispname = u.getDisplayName();
                if (dispname != null) {
                    setDisplayName(dispname);
                }

                String password = getPhoneContext().getClearTextPassword(u);
                setPwd(password);
                setProxyAddress(getRegistrationServerAddress());
                setProxyPort(getRegistrationServerPort());
            }
        } finally {
            m_root = null;
        }
    }

    public String getRegistrationServerAddress() {
        return getPhone().getSettings().getSetting(CiscoPhone.SIP).getSetting("outbound_proxy").getValue();
    }

    public String getRegistrationServerPort() {
        return getPhone().getSettings().getSetting(CiscoPhone.SIP).getSetting("outbound_proxy_port")
                .getValue();
    }

    private Setting getRoot() {
        return m_root != null ? m_root : getSettings();
    }

    private Setting getLineSet() {
        return getRoot().getSetting(PHONELINE);
    }

    private Setting getProxySet() {
        return getRoot().getSetting(PROXY);
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

    public String getShortName() {
        return getLineSet().getSetting(SHORTNAME).getValue();
    }

    public void setShortName(String userId) {
        getLineSet().getSetting(SHORTNAME).setValue(userId);
    }

    public String getProxyAddress() {
        return getProxySet().getSetting(ADDRESS).getValue();
    }

    public void setProxyAddress(String proxyIp) {
        getProxySet().getSetting(ADDRESS).setValue(proxyIp);
    }

    public String getProxyPort() {
        return getProxySet().getSetting(CiscoPhone.PORT).getValue();
    }

    public void setProxyPort(String userId) {
        getProxySet().getSetting(CiscoPhone.PORT).setValue(userId);
    }
}
