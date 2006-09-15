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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;

public class GatewayCallerAliasInfo {
    private String m_defaultCallerAlias;

    private boolean m_anonymous;

    private boolean m_ignoreUserInfo;

    private boolean m_transformUserExtension;

    private String m_addPrefix;

    private int m_keepDigits;

    /**
     * Transforms user extension into from header
     * 
     * @param user for which we are transforming extension
     * @return transformed extension which should be used, or null if there is nothing to
     *         transform
     */
    public String getTransformedNumber(User user) {
        if (!m_transformUserExtension) {
            return null;
        }
        String extension = user.getExtension();
        if (extension == null) {
            // nothing to transform
            return null;
        }
        if (m_keepDigits > 0) {
            extension = StringUtils.substring(extension, -m_keepDigits);
        }
        if (m_addPrefix != null) {
            extension = m_addPrefix + extension;
        }
        return extension;
    }

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

    public boolean isIgnoreUserInfo() {
        return m_ignoreUserInfo;
    }

    public void setIgnoreUserInfo(boolean overwriteUserInfo) {
        m_ignoreUserInfo = overwriteUserInfo;
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
        return m_transformUserExtension;
    }

    public void setTransformUserId(boolean transformUserId) {
        m_transformUserExtension = transformUserId;
    }
}
