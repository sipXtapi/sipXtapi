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

import org.sipfoundry.sipxconfig.common.User;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class LegacyContextImpl extends HibernateDaoSupport implements LegacyContext {
    public Collection getConfigSetsForUser(User user) {
        String configSetsForUser = "from UserConfigSet cs where :user in elements(cs.users)";
        return getHibernateTemplate().findByNamedParam(configSetsForUser, "user", user);
    }
}
