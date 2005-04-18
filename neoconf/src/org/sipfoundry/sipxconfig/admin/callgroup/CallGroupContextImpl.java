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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.springframework.orm.hibernate.HibernateTemplate;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Hibernate implementation of the call group context 
 */
public class CallGroupContextImpl extends HibernateDaoSupport implements CallGroupContext {
    public CallGroup loadCallGroup(Integer id) {
        return (CallGroup) getHibernateTemplate().load(CallGroup.class, id);
    }

    public void storeCallGroup(CallGroup callGroup) {
        getHibernateTemplate().saveOrUpdate(callGroup);
    }

    public void removeCallGroups(Collection ids) {
        HibernateTemplate template = getHibernateTemplate();
        Collection entities = new ArrayList(ids.size());
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Object entity = template.load(CallGroup.class, id);
            entities.add(entity);
        }
        template.deleteAll(entities);
    }
}
