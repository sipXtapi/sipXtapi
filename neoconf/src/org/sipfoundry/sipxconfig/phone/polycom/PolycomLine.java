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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Polycom business functions for line meta setting
 */
public class PolycomLine extends AbstractLine {

    public static final String FACTORY_ID = "polycom";

    public static final String REGISTRATION = "reg";

    public static final String SERVER = "server";

    public static final String ADDRESS = "address";

    public static final String PORT = "port";

    public static final String DISPLAY_NAME = "displayName";

    public static final String FIRST = "1";

    private static final int DEFAULT_SIP_PORT = 5060;
    
    /** while building model set root so getters/setting operation on this setting set*/ 
    private Setting m_root;

    /** BEAN ACCESS ONLY */
    public PolycomLine() {
    }
    
    public PolycomLine(PolycomPhone phone) {
        super(phone);
    }
    
    public Setting getSettingModel() {
        PolycomPhone polyPhone = (PolycomPhone) getPhone();
        Setting settings = polyPhone.getPolycom().getLineSettingModel().copy();

        m_root = settings;

        User u = getLineMetaData().getUser();
        if (u != null) {
            setUserId(u.getDisplayId());
            getRegistration().getSetting("auth.userId").setValue(u.getDisplayId());

            // only when there's a user to register do you set the registration server
            // although probably harmless            
            String domainName = polyPhone.getPolycom().getDnsDomain();
            setPrimaryRegistrationServerAddress(domainName);
        }

        // See pg. 125 Admin Guide/16 June 2004
        if (getLineMetaData().getPosition() == 0) {
            settings.getSetting("msg.mwi").getSetting("callBackMode").setValue("registration");
        }
        
        m_root = null;
        
        return settings;
    }
    
    private Setting getRoot() {
        return m_root != null ? m_root : getSettings();
    }

    public String getPrimaryRegistrationServerAddress() {
        return getPrimaryRegistrationServer().getSetting(ADDRESS).getValue();
    }

    public void setPrimaryRegistrationServerAddress(String address) {
        getPrimaryRegistrationServer().getSetting(ADDRESS).setValue(address);
    }

    public String getPrimaryRegistrationServerPort() {
        return getPrimaryRegistrationServer().getSetting(PORT).getValue();
    }

    public void setPrimaryRegistrationServerPort(String port) {
        getPrimaryRegistrationServer().getSetting(PORT).setValue(port);
    }
    
    private Setting getRegistration() {
        return getRoot().getSetting(REGISTRATION);
    }
    
    private Setting getPrimaryRegistrationServer() {
        return getRegistration().getSetting(SERVER).getSetting(FIRST);
    }
    
    public String getUserId() {
        return getRegistration().getSetting(DISPLAY_NAME).getValue();
    }

    public void setUserId(String userId) {
        getRegistration().getSetting(DISPLAY_NAME).setValue(userId);
    }

    public static int getSipPort(String portString) {
        int port = DEFAULT_SIP_PORT;
        if (StringUtils.isNotBlank(portString)) {
            try {
                port = Integer.parseInt(portString);
            } catch (NumberFormatException badPortNonFatal) {
                new Exception("Bad port number " + portString, badPortNonFatal).printStackTrace();
            }
        }

        return port;
    }

    public String getUri() {
        StringBuffer sb = new StringBuffer();
        sb.append("sip:").append(getUserId());
        sb.append('@').append(getPrimaryRegistrationServerAddress());
        String port = getPrimaryRegistrationServerPort();
        if (StringUtils.isNotBlank(port)) {
            sb.append(':').append(port);
        }

        return sb.toString();
    }
}