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

import java.rmi.RemoteException;

import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserServiceImpl implements UserService {
    
    private CoreContext m_coreContext;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void addUser(AddUser addUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User u = new org.sipfoundry.sipxconfig.common.User();
        User ut = addUser.getUser();
        u.setUserName(ut.getUserName());
        u.setPin(ut.getPin(), m_coreContext.getAuthorizationRealm());
        m_coreContext.saveUser(u);
    }    
}
