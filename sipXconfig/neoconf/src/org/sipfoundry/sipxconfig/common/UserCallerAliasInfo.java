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
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;

public class UserCallerAliasInfo {
    public static final String ANONYMOUS_CALLER_ALIAS = "caller-alias/anonymous-caller-alias";
    public static final String EXTERNAL_NUMBER = "caller-alias/external-number";

    private boolean m_anonymous;

    private String m_externalNumber;

    public UserCallerAliasInfo(User user) {
        m_anonymous = (Boolean) user.getSettingTypedValue(ANONYMOUS_CALLER_ALIAS);
        m_externalNumber = (String) user.getSettingTypedValue(EXTERNAL_NUMBER);
    }

    public boolean hasExternalNumber() {
        return m_anonymous || StringUtils.isNotBlank(m_externalNumber);
    }

    public String getExternalNumber(String anonymousNumber) {
        return m_anonymous ? anonymousNumber : m_externalNumber;
    }
}
