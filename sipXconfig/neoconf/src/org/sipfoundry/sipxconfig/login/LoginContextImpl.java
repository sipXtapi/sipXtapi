/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.login;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;

public class LoginContextImpl implements LoginContext {
    private CoreContext m_coreContext;

    /**
     * Returns user if credentials check out. null if user does not exist or password is wrong.
     */
    public User checkCredentials(String userName, String password) {
        User user = m_coreContext.loadUserByDisplayId(userName);
        if (user == null) {
            return null;
        }

        String pintoken = user.getPintoken();

        String encodedPassword = Md5Encoder.digestPassword(userName, m_coreContext
                .getAuthorizationRealm(), password);
        // real match
        if (encodedPassword.equals(pintoken)) {
            return user;
        }
        // also check for unencoded password - testing and compatibility support
        if (password.length() != encodedPassword.length() && password.equals(pintoken)) {
            return user;
        }

        return null;
    }

    /**
     * This function will check if logged in user has admin privilidges. Current implementation
     * just checks if user is set, if it is we assume it's NOT admin
     * 
     * @return true for user with superadmin privilidges
     */
    public boolean isAdmin(User user) {
        if (user == null) {
            return false;
        }
        // TODO: real implementation needed (checking privilidges)
        String username = user.getDisplayId();

        return "superadmin".equals(username);

    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
