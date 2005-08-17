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
     * Returns user if credentials check out.
     * Return null if the user does not exist or the password is wrong.
     * The userId may be either the userName or the user's extension.
     */
    public User checkCredentials(String userId, String password) {
        User user = m_coreContext.loadUserByUserName(userId);
        if (user == null) {
            // The userId might be the user's extension rather than userName, see if that works.
            user = m_coreContext.loadUserByExtension(userId);
            if (user == null) {
                return null;
            }
        }

        String pintoken = user.getPintoken();

        String encodedPassword = Md5Encoder.digestPassword(userId, m_coreContext
                .getAuthorizationRealm(), password);
        
        // Real match
        if (encodedPassword.equals(pintoken)) {
            return user;
        }
        
        // Special case: if the password is empty and the pintoken is empty, then declare a match.
        // We have publicized the ability for admins to reset users to have an empty password by
        // zeroing out the pintoken entry in the database.
        if (password.length() == 0 && pintoken.length() == 0) {
            return user;
        }

        return null;
    }

    /**
     * This function will check if logged in user has admin privileges. Current implementation
     * just checks if user is set, if it is we assume it's NOT admin
     * 
     * @return true for user with superadmin privileges
     */
    public boolean isAdmin(User user) {
        if (user == null) {
            return false;
        }
        // TODO: real implementation needed (checking privileges)
        String userName = user.getUserName();

        return "superadmin".equals(userName);

    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
