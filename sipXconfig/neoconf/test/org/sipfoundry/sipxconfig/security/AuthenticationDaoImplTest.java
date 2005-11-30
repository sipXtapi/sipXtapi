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

import junit.framework.TestCase;
import net.sf.acegisecurity.UserDetails;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Permission;
import org.sipfoundry.sipxconfig.common.User;

public class AuthenticationDaoImplTest extends TestCase {
    private static final String USER_NAME = "Hjelje";
    private static User s_user = null;
    
    private MockControl m_coreControl;
    private CoreContext m_coreContext;
    private AuthenticationDaoImpl m_authenticationDaoImpl;

    protected void setUp() throws Exception {
        m_coreControl = MockClassControl.createStrictControl(CoreContext.class);
        m_coreContext = (CoreContext) m_coreControl.getMock();
        m_authenticationDaoImpl = new AuthenticationDaoImpl();
        m_authenticationDaoImpl.setCoreContext(m_coreContext);
        s_user = new User() {
            public boolean hasPermission(Permission permission) {
                return true;
            }
        };
        s_user.setUserName(USER_NAME);
    }
    
    public void testLoadUserByUsername() {
        // prep the mock core context
        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        m_coreControl.setReturnValue(s_user);
        m_coreControl.replay();
        
        // load the user details
        UserDetails details = m_authenticationDaoImpl.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        assertEquals(AuthenticationDaoImpl.AUTH_USER_AND_ADMIN_ARRAY, details.getAuthorities());
        m_coreControl.verify();
    }
}
