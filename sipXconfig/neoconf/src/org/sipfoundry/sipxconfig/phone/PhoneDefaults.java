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
package org.sipfoundry.sipxconfig.phone;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Sets up phone and line objects with system defaults.
 */
public class PhoneDefaults {
    
    private static final String DEFAULT_SIP_PORT = "5060";
    
    private String m_outboundProxy;
    
    private String m_outboundProxyPort;
    
    private String m_registrationServer;
    
    private String m_registrationServerPort;
    
    private String m_tftpServer;
    
    private String m_domainName;
    
    private String m_authorizationRealm;
        
    public String getDomainName() {
        return m_domainName;
    }

    public String getOutboundProxy() {
        return m_outboundProxy;
    }

    public String getOutboundProxyPort() {
        return m_outboundProxyPort;
    }

    public String getRegistrationServer() {
        return m_registrationServer;
    }

    public String getRegistrationServerPort() {
        return m_registrationServerPort;
    }

    public String getTftpServer() {
        return m_tftpServer;
    }

    public void setOutboundProxy(String outboundProxy) {
        m_outboundProxy = outboundProxy;
    }

    public void setOutboundProxyPort(String outboundProxyPort) {
        m_outboundProxyPort = outboundProxyPort;
    }

    public void setRegistrationServer(String registrationServer) {
        m_registrationServer = registrationServer;
    }

    public void setRegistrationServerPort(String registrationServerPort) {
        m_registrationServerPort = registrationServerPort;
    }

    public void setTftpServer(String tftpServer) {
        m_tftpServer = tftpServer;
    }
    
    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }
    
    public String getAuthorizationRealm() {
        return m_authorizationRealm;
    }

    public void setAuthorizationRealm(String authorizationRealm) {
        m_authorizationRealm = authorizationRealm;
    }

    public void setPhoneDefaults(Phone phone) {
        PhoneSettings settings = (PhoneSettings) phone.getAdapter(PhoneSettings.class);
        if (settings != null) {
            settings.setOutboundProxy(m_outboundProxy);
            if (!defaultSipPort(m_outboundProxyPort)) {
                settings.setOutboundProxyPort(m_outboundProxyPort);
            }
            settings.setTftpServer(m_tftpServer);
        }
    }
    
    static boolean defaultSipPort(String port) {
        return StringUtils.isBlank(port) || DEFAULT_SIP_PORT.equals(port);
    }
    
    public void setLineDefaults(Line line, User user) {
        LineSettings settings = (LineSettings) line.getAdapter(LineSettings.class);
        if (settings != null) {
            settings.setDomainName(m_domainName);
            settings.setRegistrationServer(m_registrationServer);
            if (!defaultSipPort(m_registrationServerPort)) {
                settings.setRegistrationServerPort(m_registrationServerPort);
            }
            if (user != null) {
                settings.setAuthorizationId(user.getDisplayId());
                settings.setUserId(user.getDisplayId());
                settings.setDisplayName(user.getDisplayName());
                settings.setPassword(user.getSipPassword());
                settings.setAuthorizationRealm(m_authorizationRealm);
            }
        }
        
        if (user != null) {
            line.setUri(getUri(user));
        }
    }
    
    public String getUri(User user) {
        StringBuffer sb = new StringBuffer();
        sb.append("sip:").append(user.getDisplayId());
        sb.append('@').append(m_domainName);

        String displayName = user.getDisplayName();
        if (displayName != null) {
            sb.insert(0, "\"" + displayName + "\"<").append(">");
        }
        String uri = sb.toString();

        return uri;
    }
    
}
