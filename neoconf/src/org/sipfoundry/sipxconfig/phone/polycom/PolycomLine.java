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

import java.io.File;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.AbstractLine;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Polycom business functions for line meta setting
 */
public class PolycomLine extends AbstractLine {

    public static final String FACTORY_ID = "polycomLine";

    public static final String REGISTRATION = "reg";

    public static final String SERVER = "server";

    public static final String ADDRESS = "address";

    public static final String PORT = "port";

    public static final String DISPLAY_NAME = "displayName";

    public static final String FIRST = "1";

    private static final int DEFAULT_SIP_PORT = 5060;
    
    /** FIXME: Pull this value from Internal Dialing Rules */
    private static final String VOICEMAIL_EXT = "101";
    
    /** while building model set root so getters/setting operation on this setting set*/ 
    private Setting m_root;

    public PolycomLine() {
    }
    
    public Setting getSettingModel() {
        String systemDirectory = getPhoneContext().getSystemDirectory();
        File modelDefsFile = new File(systemDirectory + '/' 
                + PolycomPhone.FACTORY_ID + "/line.xml");
        Setting model = new XmlModelBuilder(systemDirectory).buildModel(modelDefsFile).copy();
        
        return model;
    }

    public void setDefaults(Setting settings) {

        // HACK : temporarily set root setting to trick utility methods to
        // operate on this setting set.
        m_root = settings;
        try {
            User u = getLineData().getUser();
            if (u != null) {
                setUserId(u.getDisplayId());
                setDisplayName(u.getDisplayName());
                getRegistration().getSetting("auth.userId").setValue(u.getDisplayId());
                            
                String password = getPhoneContext().getClearTextPassword(u);
                getRegistration().getSetting("auth.password").setValue(password);
    
                String domainName = getPhoneContext().getDnsDomain();
                // only when there's a user to register do you set the registration server
                // although probably harmless            
                setPrimaryRegistrationServerAddress(domainName);

                Setting mwi = settings.getSetting("msg.mwi");
                String uri = u.getDisplayId() + '@' + domainName;
                mwi.getSetting("subscribe").setValue(uri);
                mwi.getSetting("callBack").setValue(VOICEMAIL_EXT + '@' + domainName);
                mwi.getSetting("callBackMode").setValue("contact");
            }        

        } finally {        
            m_root = null;
        }
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
        return getRegistration().getSetting(ADDRESS).getValue();
    }

    public void setUserId(String userId) {
        getRegistration().getSetting(ADDRESS).setValue(userId);
    }

    public String getDisplayName() {
        return getRegistration().getSetting(DISPLAY_NAME).getValue();
    }

    public void setDisplayName(String userId) {
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
        
    /**
     * Doesn't include Display name or angle bracket, 
     * e.g. sip:user@blah.com, not "User Name"&lt;sip:user@blah.com&gt; 
     * NOTE: Unlike request URIs for REGISTER, this apparently requires the user
     * portion.  NOTE: I found this out thru trial and error.
     */
    public String getNotifyRequestUri() {
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
