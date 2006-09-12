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
    private Domain m_domain;

    /**
     * @return non-null unless test environment  
     */
    public Domain getDomain() {
        if (m_domain != null) {
            return m_domain;
        }
        Collection domains = getHibernateTemplate().findByNamedQuery("domain");
        m_domain = (Domain) DaoUtils.requireOneOrZero(domains, "named query: domain");
        if (m_domain == null) {
            throw new RuntimeException("System was not initialized properly");
        }
        return m_domain;
    }

    public void saveDomain(Domain domain) {
        getHibernateTemplate().saveOrUpdate(domain);
        setDomain(domain);
    }
    
    public void setDomain(Domain domain) {
        m_domain = domain;
    }
}
