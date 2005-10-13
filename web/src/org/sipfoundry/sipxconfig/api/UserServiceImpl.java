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
package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class UserServiceImpl implements UserService {

    private CoreContext m_coreContext;

    public void createUser(String userName, String pin) {
        User u = new User();
        u.setUserName(userName);
        u.setPin(pin, m_coreContext.getAuthorizationRealm());
        m_coreContext.saveUser(u);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
