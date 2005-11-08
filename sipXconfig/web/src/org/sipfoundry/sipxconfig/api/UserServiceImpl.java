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
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class UserServiceImpl implements UserService {
    
    /** TODO: Remove this */
    private static final int PAGE_SIZE = 1000;
    
    private CoreContext m_coreContext;
    
    private SettingDao m_settingDao;
    
    private UserBuilder m_userBuilder;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setUserBuilder(UserBuilder userTranslater) {
        m_userBuilder = userTranslater;
    }
    
    public void addUser(AddUser addUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User neoUser = new org.sipfoundry.sipxconfig.common.User();
        User soapUser = addUser.getUser();        
        m_userBuilder.fromApi(soapUser, neoUser);
        String[] groups = addUser.getGroup();
        String resourceId = org.sipfoundry.sipxconfig.common.User.GROUP_RESOURCE_ID;
        for (int i = 0; groups != null && i < groups.length; i++) {
            Group g = m_settingDao.getGroupCreateIfNotFound(resourceId, groups[i]);
            neoUser.addGroup(g);
        }
        m_coreContext.saveUser(neoUser);
    }
    
    public FindUserResponse findUser(FindUser findUser) throws RemoteException {
        FindUserResponse response = new FindUserResponse();
        List users = m_coreContext.loadUsersByPage(findUser.getByName(),
                null, 0, PAGE_SIZE, null, true);
        
        User[] arrayOfUsers = (User[]) ApiBeanUtil.toApiArray(m_userBuilder, users.toArray(), 
                User.class);
        response.setUsers(arrayOfUsers);
        
        return response;
    }

    public void deleteUser(DeleteUser deleteUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User user = requireUser(deleteUser.getUserName());
        m_coreContext.deleteUser(user);
    }

    public void editUser(EditUser editUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User user = requireUser(editUser.getUserName());
        ApiBeanUtil.setProperties(user, editUser.getProperties());
        m_coreContext.saveUser(user);
    }
    
    private org.sipfoundry.sipxconfig.common.User requireUser(String userName) {
        org.sipfoundry.sipxconfig.common.User user = m_coreContext.loadUserByUserName(userName);
        if (user == null) {
            throw new IllegalArgumentException("user not found with user name " + userName);
        }
        return user;
    }
}
