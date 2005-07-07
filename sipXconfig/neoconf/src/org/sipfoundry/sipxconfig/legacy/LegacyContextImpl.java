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

import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class LegacyContextImpl extends HibernateDaoSupport implements LegacyContext {
    
    public UserConfigSet getConfigSetForUser(User user) {
        String configSetsForUser = "from UserConfigSet cs where :user in elements(cs.users)";
        Collection configs = getHibernateTemplate().findByNamedParam(configSetsForUser, "user", user);
        if (configs.size() > 1) {
            throw new RuntimeException("Unexpected multiple config sets for user " + user.getDisplayId());
        }
        return configs.isEmpty() ? null : (UserConfigSet) configs.iterator().next();
    }

    /**
     * At the moment it only looks for user config set, we also need to take user's groups into
     * account.
     */
    public boolean checkUserPermission(User user, Permission permission) {
        UserConfigSet cs = getConfigSetForUser(user);
        return cs != null && cs.hasPermission(permission);
    }
}
