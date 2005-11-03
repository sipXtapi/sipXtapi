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
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserServiceImpl implements UserService {
    
    private CoreContext m_coreContext;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void addUser(AddUser addUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User u = new org.sipfoundry.sipxconfig.common.User();
        User ut = addUser.getUser();
        fromSoap(u, ut);
        m_coreContext.saveUser(u);
    }

    public User[] findUser(FindUser findUser) throws RemoteException {
        List users = m_coreContext.loadUsersByPage(findUser.getByName(), 
                null, 0, 1000, null, true);
        
        User[] arrayOfUsers = new User[users.size()];
        for (int i = 0; i < users.size(); i++) {
            org.sipfoundry.sipxconfig.common.User u = (org.sipfoundry.sipxconfig.common.User) users.get(i);
            arrayOfUsers[i] = new User();
            toSoap(u, arrayOfUsers[i]);
        }
        
        return arrayOfUsers;
    }    
    
    void toSoap(org.sipfoundry.sipxconfig.common.User from, User to) {
        to.setUserName(from.getUserName());
        to.setPintoken(from.getPintoken());        
    }
    
    void fromSoap(org.sipfoundry.sipxconfig.common.User to, User from) {
        to.setUserName(from.getUserName());
        to.setPin(from.getPin(), m_coreContext.getAuthorizationRealm());                
    }
}
