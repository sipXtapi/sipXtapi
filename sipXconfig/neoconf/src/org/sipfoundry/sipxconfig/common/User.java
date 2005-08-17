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
package org.sipfoundry.sipxconfig.common;

import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends BeanWithGroups {

    private String m_firstName;

    private String m_sipPassword;

    private String m_pintoken;

    private String m_lastName;

    private String m_userName;

    private String m_extension;
    
    /** 
     * Return the pintoken, which is the hash of the user's PIN.
     * The PIN itself is private to the user.  To keep the PIN secure, we don't store it.
     */
    public String getPintoken() {
        // for robustness, return an empty string rather than null
        if (m_pintoken == null) {
            m_pintoken = new String();
        }
        
        return m_pintoken;
    }

    /**
     * Set the pintoken, which is the hash of the user's PIN.
     * This method is only for prevent the pintoken from being nulluse by Hibernate.  Call setPin to change the PIN.
     */
    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    /**
     * Set the PIN, protecting it under a security realm.
     * The PIN is private to the user.  To keep the PIN secure, we don't store it.
     * Instead we store the "pintoken", which is a hash of the PIN.
     * 
     * @param pin PIN
     * @param realm security realm
     */
    public void setPin(String pin, String realm) {
        String pin2 = (String) ObjectUtils.defaultIfNull(pin, StringUtils.EMPTY);   // handle null pin
        setPintoken(Md5Encoder.digestPassword(m_userName, realm, pin2));
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }

    public String getSipPasswordHash(String realm) {
        String password = (String) ObjectUtils.defaultIfNull(m_sipPassword, StringUtils.EMPTY);
        return Md5Encoder.digestPassword(m_userName, realm, password);
    }

    public void setSipPassword(String password) {
        m_sipPassword = password;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getDisplayName() {
        StringBuffer sb = new StringBuffer();
        delimAppend(sb, m_firstName, ' ');
        delimAppend(sb, m_lastName, ' ');

        return sb.length() == 0 ? null : sb.toString();
    }

    private void delimAppend(StringBuffer sb, String s, char delim) {
        if (StringUtils.isNotBlank(s)) {
            if (sb.length() != 0) {
                sb.append(delim);
            }
            sb.append(s);
        }
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getUri(String domainName) {
        StringBuffer uri = new StringBuffer();

        delimAppend(uri, m_firstName, ' ');
        delimAppend(uri, m_lastName, ' ');

        boolean needsWrapping = uri.length() > 0;

        if (needsWrapping) {
            uri.append("<");
        }

        uri.append("sip:");
        uri.append(m_userName);
        uri.append("@" + domainName);

        if (needsWrapping) {
            uri.append(">");
        }

        return uri.toString();
    }

    public List getAliases(String domainName) {
        if (StringUtils.isBlank(m_extension)) {
            return Collections.EMPTY_LIST;
        }
        final String identity = AliasMapping.createUri(m_extension, domainName);
        final String contact = getUri(domainName);
        AliasMapping mapping = new AliasMapping(identity, contact);
        return Collections.singletonList(mapping);
    }
    
    /**
     * Check if a user has a specific permission
     */
    public boolean hasPermission(Permission permission) {        
        Setting setting = getSettings().getSetting(permission.getSettingPath());
        if (setting == null) {
            throw new IllegalArgumentException("Setting " 
                    + permission.getName() + " does not exist in user setting model");
        }
        boolean enabled = Permission.isEnabled(setting.getValue());        
        return enabled;
    }
}
