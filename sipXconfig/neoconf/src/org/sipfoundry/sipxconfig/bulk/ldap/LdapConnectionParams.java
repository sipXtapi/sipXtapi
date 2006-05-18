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
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.text.MessageFormat;

import org.apache.commons.lang.StringUtils;

/**
 * Used to store LDAP connections in the DB LdapConnectionParams
 */
public class LdapConnectionParams {
    private String m_host;
    private int m_port = 389;
    private String m_principal;
    private String m_secret;

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public int getPort() {
        return m_port;
    }

    public void setPort(int port) {
        m_port = port;
    }

    public String getPrincipal() {
        return m_principal;
    }

    public void setPrincipal(String principal) {
        m_principal = principal;
    }

    public String getSecret() {
        return m_secret;
    }

    public void setSecret(String secret) {
        m_secret = secret;
    }

    private String getUrl() {
        return MessageFormat.format("ldap://{0}:{1}", m_host, m_port);
    }

    public void applyToTemplate(JndiLdapTemplate template) {
        template.setProviderUrl(getUrl());
        template.setSecurityPrincipal(m_principal);
        template.setSecurityCredentials(m_secret);
        String authentication = StringUtils.isEmpty(m_secret) ? "none" : "basic";
        template.setSecurityAuthentication(authentication);
    }
}
