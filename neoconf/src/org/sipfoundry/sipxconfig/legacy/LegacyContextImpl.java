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
import java.util.Iterator;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class LegacyContextImpl extends HibernateDaoSupport implements LegacyContext {
    public Collection getConfigSetsForUser(User user) {
        String configSetsForUser = "from UserConfigSet cs where :user in elements(cs.users)";
        return getHibernateTemplate().findByNamedParam(configSetsForUser, "user", user);
    }

    /**
     * At the moment it only looks for user config set, we also need to take user's groups into
     * account.
     */
    public boolean checkUserPermission(User user, Permission permission) {
        Collection configSetsForUser = getConfigSetsForUser(user);
        for (Iterator i = configSetsForUser.iterator(); i.hasNext();) {
            UserConfigSet cs = (UserConfigSet) i.next();
            if (cs.hasPermission(permission)) {
                return true;
            }
        }
        return false;
    }
}
