/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

import java.util.Collection;
import java.util.Iterator;

import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use Hibernate to r/w object to database for device related items
 */
public class CoreDaoImpl extends HibernateDaoSupport implements CoreDao {
    
    public void storeUser(User user) {
        getHibernateTemplate().saveOrUpdate(user);        
    }

    public void storeLogicalPhone(LogicalPhone logicalPhone) {
        getHibernateTemplate().saveOrUpdate(logicalPhone);        
    }
    
    public void storeOrganization(Organization org) {
        getHibernateTemplate().saveOrUpdate(org);        
    }

    public void storeLine(Line line) {
        getHibernateTemplate().saveOrUpdate(line);        
    }

    public LogicalPhone loadLogicalPhone(int id) {
        return (LogicalPhone) getHibernateTemplate().load(LogicalPhone.class, new Integer(id));
    }
    
    public Line loadLine(User user, int position) {
        String query = "from Line where user_id = " + user.getId()
                + " and position = " + position;
        Collection line = getHibernateTemplate().find(query);
        
        return (Line) requireOneOrZero(line, query);
    }
    
    /**
     * @throws IllegalStateException if more than one item in collection 
     * @param c
     * @param query
     * @return
     */
    Object requireOneOrZero(Collection c, String query) {
        if (c.size() > 2) {
            // DatabaseCorruptionExection ?
            StringBuffer error = new StringBuffer().append("read ").append(c.size())
                    .append(" and expected zero or one. query=").append(query);
            throw new IllegalStateException(error.toString());
        }        
        Iterator i = c.iterator();
        
        return (i.hasNext() ? c.iterator().next() : null);
    }
    
    public Organization loadOrganization(int id) {
        return (Organization) getHibernateTemplate().load(Organization.class, new Integer(id));        
    }
    
    public User loadUser(int id) {
        return (User) getHibernateTemplate().load(User.class, new Integer(id));        
    }
    
    public void deleteOrganization(Organization org) {
        Collection phones = getHibernateTemplate().find("from LogicalPhone where Organization = ?", 
                new Integer(org.getId()));
        Iterator iphones = phones.iterator();
        while (iphones.hasNext()) {
            LogicalPhone phone = (LogicalPhone) iphones.next();
            getHibernateTemplate().delete(phone);
        }
        
        
        getHibernateTemplate().delete(org);
    }
}
