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
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public class UserServiceImpl implements UserService {

    private CoreContext m_coreContext;
    
    private SettingDao m_settingDao;
        
    public void createUser(String userName, String pin, String firstName, String lastName, 
            String aliases, String userGroups) {
        User u = new User();
        u.setUserName(userName);
        u.setPin(pin, m_coreContext.getAuthorizationRealm());
        u.setFirstName(firstName);
        u.setLastName(lastName);
        u.setAliasesString(aliases);
        String[] groups = User.groupNamesFromString(userGroups);
        for (int i = 0; groups != null && i < groups.length; i++) {            
            Group g = m_settingDao.getGroupByName(User.GROUP_RESOURCE_ID, groups[i].trim());
            u.addGroup(g);
        }
        m_coreContext.saveUser(u);
        
    }
    
    public void deleteUser(String userName) {
        User u = requireUser(userName);
        m_coreContext.deleteUser(u);
    }
    
    private User requireUser(String username) {
        User u = m_coreContext.loadUserByUserName(username);
        if (u == null) {
            throw new IllegalArgumentException("Could not find user with username '" + username + "'");
        }
        
        return u;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }    
}
