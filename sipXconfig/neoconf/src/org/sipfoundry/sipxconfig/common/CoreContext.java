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

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
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

    public void deleteUsers(Collection users);

    public User loadUser(Integer id);
    
    public List loadUsers();
    
    public List loadUserByTemplateUser(User template);

    public User loadUserByUserName(String userName);

    public User loadUserByExtension(String extension);
    
    public String getAuthorizationRealm();
    
    public String getDomainName();
    
    public void clear();
    
    public boolean checkUserPermission(User user, Permission p);
    
    public Group loadRootUserGroup();
    
    public List getUserGroupsWithoutRoot();

    public List getUserGroups();
    
    /**
     * Model of all user settings
     */
    public Setting getUserSettingsModel();

    public List getUserAliases();

    public void setProcessContext(SipxProcessContext processContext);

    public SipxProcessContext getProcessContext();
}
