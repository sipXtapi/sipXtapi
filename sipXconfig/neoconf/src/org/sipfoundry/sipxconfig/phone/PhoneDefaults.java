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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Sets up phone and line objects with system defaults.
 */
public class PhoneDefaults {

    private static final String DEFAULT_SIP_PORT = "5060";

    private String m_tftpServer;

    private String m_profileRootUrl;

    private String m_domainName;
    
    private String m_fullyQualifiedDomainName; 

    private String m_authorizationRealm;

    private DialPlanContext m_dialPlanContext;

    /** see config.defs PROXY_SERVER_ADDR */
    private String m_proxyServerAddr;

    /** see config.defs PROXY_SERVER_SIP_PORT */
    private String m_proxyServerSipPort;

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public String getDomainName() {
        return m_domainName;
    }

    public String getTftpServer() {
        return m_tftpServer;
    }

    public void setProxyServerAddr(String proxyServerAddr) {
        m_proxyServerAddr = proxyServerAddr;
    }

    /**
     * Only use this function when IP address of the proxy is needed. 
     * In most cases you should be able to use SIP domain name
     */
    public String getProxyServerAddr() {
        return m_proxyServerAddr;
    }

    public void setProxyServerSipPort(String proxyServerSipPort) {
        m_proxyServerSipPort = proxyServerSipPort;
    }

    /**
     * Only use this function when port of the the proxy is needed. 
     * In most cases you should be able to use SIP domain name
     */
    public String getProxyServerSipPort() {
        return m_proxyServerSipPort;
    }

    /**
     * URL where phone profiles are delivered from apache web server.
     * 
     * @return generated url if not set
     */
    public String getProfileRootUrl() {
        if (m_profileRootUrl != null) {
            return m_profileRootUrl;
        }

        StringBuffer url = new StringBuffer();
        url.append("http://").append(getFullyQualifiedDomainName()).append(":8090");
        url.append("/phone/profile/docroot");

        return url.toString();
    }

    public void setProfileRootUrl(String profileUrl) {
        m_profileRootUrl = profileUrl;
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
            settings.setOutboundProxy(m_domainName);
            settings.setTftpServer(m_tftpServer);
            String voiceMail = getVoiceMail();
            if (StringUtils.isNotBlank(voiceMail)) {
                settings.setVoiceMailNumber(voiceMail);
            }
        }
    }

    public String getVoiceMail() {
        if (m_dialPlanContext == null) {
            return InternalRule.DEFAULT_VOICEMAIL;
        }

        return m_dialPlanContext.getVoiceMail();
    }

    static boolean defaultSipPort(String port) {
        return StringUtils.isBlank(port) || DEFAULT_SIP_PORT.equals(port);
    }

    public void setLineDefaults(Line line, User user) {
        if (user != null) {
            LineSettings settings = (LineSettings) line.getAdapter(LineSettings.class);
            if (settings != null) {
                settings.setDomainName(m_domainName);
                settings.setRegistrationServer(m_domainName);
                settings.setAuthorizationId(user.getUserName());
                settings.setUserId(user.getUserName());
                settings.setDisplayName(user.getDisplayName());
                settings.setPassword(user.getSipPassword());
                settings.setAuthorizationRealm(m_authorizationRealm);
                settings.setVoiceMail(getVoiceMail());
            }
        }
    }

    public String getUri(LineSettings settings) {        
        StringBuffer sb = new StringBuffer();
        sb.append("sip:").append(settings.getUserId());
        
        // HACK: Although getDomainName may make more sense, in practice 
        // phones do not store domain name and therefore would return
        // null.  This is because LineSettings is typically backed by 
        // SettingBeanAdapter and setX(foo) does not always mean foo == getX()
        // if there's no mapping. All phones need a registration server.
        // Real fix might be to remove SettingBeanAdapter with something more
        // flexible
        sb.append('@').append(settings.getRegistrationServer());

        String displayName = settings.getDisplayName();
        if (displayName != null) {
            sb.insert(0, "\"" + displayName + "\"<").append(">");
        }
        String uri = sb.toString();

        return uri;
    }

    public String getFullyQualifiedDomainName() {
        return m_fullyQualifiedDomainName;
    }

    public void setFullyQualifiedDomainName(String fullyQualifiedDomainName) {
        m_fullyQualifiedDomainName = fullyQualifiedDomainName;
    }
}
