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
package org.sipfoundry.sipxconfig.common;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;


/**
 * CoreContext
 */
public interface CoreContext extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "coreContext";
    
    public static final Integer UNSAVED_ID = new Integer(-1);

    public void saveUser(User user);
    
    public void deleteUser(User user);

    public void deleteUsers(Collection usersIds);

    public User loadUser(Integer id);
    
    public List loadUsers();
    
    public List loadUserByTemplateUser(User template);

    public User loadUserByUserName(String userName);
    
    public User loadUserByAlias(String userName);

    public User loadUserByUserNameOrAlias(String userNameOrAlias);
    
    public String getAuthorizationRealm();
    
    public String getDomainName();
    
    public void setDomainName(String domainName);
    
    public void clear();
    
    public List getUserGroups();
    
    /**
     * Model of all user settings
     */
    public Setting getUserSettingsModel();

    public List getUserAliases();

    public Collection getGroupMembers(Group group);
    
    /**
     * Called internally by boot-up process
     */
    public void createAdminGroupAndInitialUserTask();
}
