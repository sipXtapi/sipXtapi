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
package org.sipfoundry.sipxconfig.site;

import org.mortbay.http.HttpRequest;
import org.mortbay.http.UserPrincipal;
import org.mortbay.http.UserRealm;

/**
 * Simple implementation of authentication realm for unit testing
 * Currently any user is authenticated as "superadmin" with all the roles.
 * One can add other users to test if role based authentication works correctly
 */
class JettyUserRealm implements UserRealm {    
    private static final String[] ADMIN_ROLES = {
        "ADMIN", "END_USER", "SUPER"
    };
    
    static class User implements UserPrincipal {
        private String[] m_roles;

        public User(String[] roles) {
            m_roles = roles;
        }
        
        public boolean isAuthenticated() {
            return true;
        }
    
        /**
         * Any role is a good role...
         */
        public boolean isUserInRole(String role) {
            for (int i = 0; i < m_roles.length; i++) {
                if (m_roles[i].equals(role)) {
                    return true;
                }
            }
            return false;
        }
    
        public String getName() {
            return "admin";
        }
    }

    UserPrincipal m_admin = new User(ADMIN_ROLES);

    public String getName() {
        return "PGS";
    }

    public UserPrincipal authenticate(String arg0, Object arg1, HttpRequest request) {
        return m_admin;
    }

    public void disassociate(UserPrincipal up) {
        // empty
    }

    public UserPrincipal pushRole(UserPrincipal arg0, String arg1) {
        throw new UnsupportedOperationException();
    }

    public UserPrincipal popRole(UserPrincipal arg0) {
        throw new UnsupportedOperationException();
    }

    public void logout(UserPrincipal arg0) {
        // empty
    }
}