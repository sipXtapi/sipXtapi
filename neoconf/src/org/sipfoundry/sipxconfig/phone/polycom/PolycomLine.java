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

    /** BEAN ACCESS ONLY */
    public PolycomLine() {
    }
    
    public PolycomLine(PolycomPhone phone) {
        super(phone);
    }
    
    public Setting getSettingModel() {
        PolycomPhone polyPhone = (PolycomPhone) getPhone();
        Setting settings = polyPhone.getPolycom().getLineSettingModel().copy();

        User u = getLineMetaData().getUser();
        if (u != null) {
            Setting reg = settings.getSetting(REGISTRATION);
            reg.getSetting(DISPLAY_NAME).setValue(u.getDisplayId());
            reg.getSetting("auth.userId").setValue(u.getDisplayId());

            // only when there's a user to register do you set the registration server
            // although probably harmless
            String domainName = polyPhone.getPolycom().getDnsDomain();
            reg.getSetting(SERVER).getSetting(FIRST).getSetting(ADDRESS).setValue(domainName);            
        }

        // See pg. 125 Admin Guide/16 June 2004
        if (getLineMetaData().getPosition() == 0) {
            settings.getSetting("msg.mwi").getSetting("callBackMode").setValue("registration");
        }
        
        return settings;
    }

    public String getPrimaryRegistrationServerAddress() {
        // FIXME: ugly
        return getSettings().getSetting(REGISTRATION).getSetting(SERVER).getSetting(FIRST)
                .getSetting(ADDRESS).getValue();
    }

    public void setPrimaryRegistrationServerAddress(String address) {
        // FIXME: ugly
        getSettings().getSetting(REGISTRATION).getSetting(SERVER).getSetting(FIRST).getSetting(
                ADDRESS).setValue(address);
    }

    public String getPrimaryRegistrationServerPort() {
        // FIXME: ugly
        return getSettings().getSetting(REGISTRATION).getSetting(SERVER).getSetting(FIRST)
                .getSetting(PORT).getValue();
    }

    public void setPrimaryRegistrationServerPort(String address) {
        // FIXME: ugly
        getSettings().getSetting(REGISTRATION).getSetting(SERVER).getSetting(FIRST).getSetting(PORT)
                .setValue(address);
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
        sb.append(getLineMetaData().getDisplayLabel());
        sb.append('@').append(getPrimaryRegistrationServerAddress());
        String port = getPrimaryRegistrationServerPort();
        if (StringUtils.isNotBlank(port)) {
            sb.append(':').append(port);
        }

        return sb.toString();
    }
}