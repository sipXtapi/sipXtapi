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

import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;

public class UserServiceSoapBindingImpl implements UserService {
    
    private CoreManager m_coreContext;
    
    public void setCoreContext(CoreManager coreContext) {
        m_coreContext = coreContext;
    }

    public void addUser(AddUser addUser) throws RemoteException {
        System.out.println("User " + addUser.getUserName()
                + " with pintoken " + addUser.getPin());
        User u = new User();
        u.setUserName(addUser.getUserName());
        u.setPin(addUser.getPin(), m_coreContext.getAuthorizationRealm());
        m_coreContext.saveUser(u);
    }
    
}
