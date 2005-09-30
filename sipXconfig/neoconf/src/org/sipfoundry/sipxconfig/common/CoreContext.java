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

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * CoreContext
 */
public interface CoreContext extends DataObjectSource, AliasProvider {

    public static final String CONTEXT_BEAN_NAME = "coreContext";

    public static final Integer UNSAVED_ID = new Integer(-1);

    public void saveUser(User user);

    public void deleteUser(User user);

    public void deleteUsers(Collection usersIds);

    public User loadUser(Integer id);

    public List loadUsers();
    
    public int getUsersCount();
    
    public int getUsersInGroupCount(Integer groupId);
    
    public List loadUsersByPage(Integer groupId, int page, int pageSize, String orderBy, boolean orderAscending);

    public List loadUserByTemplateUser(User template);

    public List loadUserByTemplateUser(User userTemplate, boolean matchUserNameOrAlias);

    public User loadUserByUserName(String userName);

    public User loadUserByAlias(String userName);

    public User loadUserByUserNameOrAlias(String userNameOrAlias);
    
    /**
     * Check whether the user has a username or alias that collides with an existing username
     * or alias.  Check for internal collisions as well, for example, the user has an alias
     * that is the same as the username.  (Duplication within the aliases is not possible 
     * because the aliases are stored as a Set.)
     * If there is a collision, then return the bad name (username or alias).
     * Otherwise return null.
     * If there are multiple collisions, then it's arbitrary which name is returned.
     * 
     * @param user user to test
     * @return name that collides
     */
    public String checkForDuplicateNameOrAlias(User user);

    public String getAuthorizationRealm();

    public String getDomainName();

    public void setDomainName(String domainName);

    public void clear();

    public List getGroups();

    /**
     * Model of all user settings
     */
    public Setting getUserSettingsModel();

    public Collection getGroupMembers(Group group);

    /**
     * Called to create a superadmin user with an empty password,
     * to recover from a situation where there are no admin users in the DB
     */
    public void createAdminGroupAndInitialUserTask();
    
    /**
     * Called by the bootstrap page to create the superadmin user, giving it
     * the specified pin
     * 
     * @param pin
     */
    public void createAdminGroupAndInitialUser(String pin);
}
