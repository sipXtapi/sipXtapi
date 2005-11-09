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
package org.sipfoundry.sipxconfig.search;

import org.hibernate.SessionFactory;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class IndexManagerImpl extends HibernateDaoSupport implements IndexManager {
    // TODO: inject externally
    private final Class[] m_indexedClasses = {
        User.class, Phone.class, CallGroup.class, DialingRule.class, Bridge.class,
        Conference.class, ParkOrbit.class
    };

    private Indexer m_indexer;

    /**
     * Loads all entities to be indexed.
     */
    public void indexAll() {
        m_indexer.open();
        // load all classes that need to be indexed
        for (int i = 0; i < m_indexedClasses.length; i++) {
            getHibernateTemplate().loadAll(m_indexedClasses[i]);
        }
        m_indexer.close();
    }

    public void setIndexer(Indexer indexer) {
        m_indexer = indexer;
    }

    /**
     * Make sure that we always create a new session.
     * 
     * We need to use our entity interceptor for indexing to work.
     */
    protected HibernateTemplate createHibernateTemplate(SessionFactory sessionFactory) {
        HibernateTemplate hibernate = super.createHibernateTemplate(sessionFactory);
        hibernate.setAlwaysUseNewSession(true);
        return hibernate;
    }
}
