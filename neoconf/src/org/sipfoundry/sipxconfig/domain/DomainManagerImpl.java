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
package org.sipfoundry.sipxconfig.domain;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class DomainManagerImpl extends SipxHibernateDaoSupport implements DomainManager {

    public List getDomains() {
        List domains = getHibernateTemplate().loadAll(Domain.class);
        return domains;
    }
}
