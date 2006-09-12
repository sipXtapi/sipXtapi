/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager {

    /**
     * @return non-null unless test environment  
     */
    public Domain getDomain() {
        Collection domains = getHibernateTemplate().findByNamedQuery("domain");
        Domain d = (Domain) DaoUtils.requireOneOrZero(domains, "named query: domain");
        if (d == null) {
            throw new RuntimeException("System was not initialized properly");
        }
        return d;
    }

    public void saveDomain(Domain domain) {
        getHibernateTemplate().saveOrUpdate(domain);
    }
}
