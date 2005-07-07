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
package org.sipfoundry.sipxconfig.legacy;

import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserGroup;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class LegacyContextImpl extends HibernateDaoSupport implements LegacyContext {
    
    public UserConfigSet getConfigSetForUser(User user) {
        List configSets = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "findConfigSetsForUser", "userId", user.getId());
        if (configSets.size() > 1) {
            throw new RuntimeException("Unexpected multiple config sets for user: " + user.getDisplayId());
        }        
        return configSets.isEmpty() ? null : (UserConfigSet) configSets.get(0);
    }

    public UserConfigSet getConfigSetForUserGroup(UserGroup ug) {
        List configSets = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "findConfigSetsForUserGroup", "userGroupId", ug.getId());
        if (configSets.size() > 1) {
            throw new RuntimeException("Unexpected multiple config sets for user group: " + ug.getId());
        }        
        return configSets.isEmpty() ? null : (UserConfigSet) configSets.get(0);
    }

    /**
     * Check for user permission, if user permission is not set checks for groups permission
     */
    public boolean checkUserPermission(User user, Permission permission) {
        UserConfigSet cs = getConfigSetForUser(user);
        if (null == cs) {
            return false;
        }
        Boolean userEnabled = cs.hasPermission(permission);
        if (null != userEnabled) {
            return userEnabled.booleanValue();
        }
        Integer ugId = user.getUserGroupId();
        return checkGroupPermission(ugId, permission);
    }

    /**
     * Walks the chain of the users groups looking for user permission. Only checks parent group
     * if permission is not set in current group. If group does not have a parent, and permission
     * is not present in the group returns false.
     * 
     * @param groupId id of the group for which we are starting the permission checking
     * @return true if group all any of its parent has the permission enabled, false if permission
     *         is not set in the group or any of its ancesstor or if it is set to false
     */
    boolean checkGroupPermission(Integer groupId, Permission permission) {
        boolean hasPermission = false;

        for (Integer id = groupId; null != id;) {
            UserGroup ug = (UserGroup) getHibernateTemplate().load(UserGroup.class, id);
            UserConfigSet cs = getConfigSetForUserGroup(ug);
            if (null != cs) {
                Boolean permissionPresent = cs.hasPermission(permission);
                if (null != permissionPresent) {
                    hasPermission = permissionPresent.booleanValue();
                    break;
                }
            }
            id = ug.getParentGroupId();
        }

        return hasPermission;
    }
}
