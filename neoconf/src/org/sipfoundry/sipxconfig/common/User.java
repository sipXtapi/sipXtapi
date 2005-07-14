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

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends BeanWithGroups {

    private String m_firstName;

    private String m_sipPassword;

    private String m_pintoken;

    private String m_lastName;

    private String m_displayId;

    private String m_extension;

    public String getPintoken() {
        return m_pintoken;
    }

    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    public String getPintokenHash(String realm) {
        String password = (String) ObjectUtils.defaultIfNull(m_pintoken, StringUtils.EMPTY);
        if (password.length() == Md5Encoder.LEN) {
            return password;
        }
        return Md5Encoder.digestPassword(m_displayId, realm, password);
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
        return Md5Encoder.digestPassword(m_displayId, realm, password);
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

    public String getDisplayId() {
        return m_displayId;
    }

    public void setDisplayId(String displayId) {
        m_displayId = displayId;
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
        uri.append(m_displayId);
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
}
