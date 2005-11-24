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
package org.sipfoundry.sipxconfig.security;

import net.sf.acegisecurity.GrantedAuthority;
import net.sf.acegisecurity.GrantedAuthorityImpl;
import net.sf.acegisecurity.UserDetails;
import net.sf.acegisecurity.providers.dao.AuthenticationDao;
import net.sf.acegisecurity.providers.dao.UsernameNotFoundException;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class AuthenticationDaoImpl implements AuthenticationDao {
    /** Whether dummy admin user is enabled. For use only by unit tests! */
    private static boolean s_dummyAdminUserEnabled;
    private static final String DUMMY_ADMIN_USER_NAME = "dummyAdminUserNameForTestingOnly";
    
    // granted authorities
    private static final GrantedAuthority AUTH_USER = new GrantedAuthorityImpl(User.ROLE_USER);
    private static final GrantedAuthority AUTH_ADMIN = new GrantedAuthorityImpl(User.ROLE_ADMIN);
    private static final GrantedAuthority[] AUTH_USER_ARRAY = new GrantedAuthority[] {AUTH_USER};
    private static final GrantedAuthority[] AUTH_USER_AND_ADMIN_ARRAY =
        new GrantedAuthority[] {AUTH_USER, AUTH_ADMIN};
    
    private CoreContext m_coreContext;

    public UserDetails loadUserByUsername(String userNameOrAlias) {
        if (isDummyAdminUserEnabled() && userNameOrAlias.equals(DUMMY_ADMIN_USER_NAME)) {
            return loadDummyAdminUser();
        }
        
        User user = m_coreContext.loadUserByUserNameOrAlias(userNameOrAlias);
        if (user == null) {
            throw new UsernameNotFoundException(userNameOrAlias);
        }
        
        // All users are granted ROLE_USER.  Only admins get ROLE_ADMIN.
        boolean isAdmin = user.isAdmin();
        GrantedAuthority[] authorities = isAdmin ? AUTH_USER_AND_ADMIN_ARRAY : AUTH_USER_ARRAY;
        
        UserDetailsImpl details = new UserDetailsImpl(user, userNameOrAlias, authorities);
        return details;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /** Return true if dummy admin user is enabled, false otherwise */
    public static boolean isDummyAdminUserEnabled() {
        return s_dummyAdminUserEnabled;
    }

    /** Enable dummy admin user. For use by unit tests only! */
    public static void setDummyAdminUserEnabled(boolean dummyAdminUserEnabled) {
        s_dummyAdminUserEnabled = dummyAdminUserEnabled;
    }
    
    private UserDetails loadDummyAdminUser() {
        User testUser = new User();
        testUser.setUserName(DUMMY_ADMIN_USER_NAME);
        testUser.setPintoken("");
        
        // give the dummy admin user full privileges
        UserDetailsImpl details = new UserDetailsImpl(testUser, DUMMY_ADMIN_USER_NAME, AUTH_USER_AND_ADMIN_ARRAY);
        
        return details;        
    }
}
