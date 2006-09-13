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
package org.sipfoundry.sipxconfig.gateway;

public class GatewayCallerAliasInfo {
    private String m_defaultCallerAlias;

    private boolean m_anonymous;

    private boolean m_overwriteUserInfo;

    private boolean m_transformUserId;

    private String m_addPrefix;

    private int m_keepDigits;

    public String getDefaultCallerAlias() {
        return m_defaultCallerAlias;
    }

    public void setDefaultCallerAlias(String defaultCallerAlias) {
        m_defaultCallerAlias = defaultCallerAlias;
    }

    public boolean isAnonymous() {
        return m_anonymous;
    }

    public void setAnonymous(boolean anonymous) {
        m_anonymous = anonymous;
    }

    public boolean isOverwriteUserInfo() {
        return m_overwriteUserInfo;
    }

    public void setOverwriteUserInfo(boolean overwriteUserInfo) {
        m_overwriteUserInfo = overwriteUserInfo;
    }

    public String getAddPrefix() {
        return m_addPrefix;
    }

    public void setAddPrefix(String addPrefix) {
        m_addPrefix = addPrefix;
    }

    public int getKeepDigits() {
        return m_keepDigits;
    }

    public void setKeepDigits(int keepDigits) {
        m_keepDigits = keepDigits;
    }

    public boolean isTransformUserId() {
        return m_transformUserId;
    }

    public void setTransformUserId(boolean transformUserId) {
        m_transformUserId = transformUserId;
    }
}
