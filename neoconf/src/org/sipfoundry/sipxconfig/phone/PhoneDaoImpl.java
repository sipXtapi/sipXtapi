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
package org.sipfoundry.sipxconfig.phone;

import java.util.Iterator;

import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use Hibernate to r/w object to database for device related items
 */
public class PhoneDaoImpl extends HibernateDaoSupport implements PhoneDao {
    
    public void flush() {
        getHibernateTemplate().flush();
    }
    
    public User loadUser(int id) {
        return (User) getHibernateTemplate().load(User.class, new Integer(id));        
    }    
    
    public void storeEndpoint(Endpoint endpoint) {
        getHibernateTemplate().saveOrUpdate(endpoint);        
    }
    
    public void deleteEndpoint(Endpoint endpoint) {
        getHibernateTemplate().delete(endpoint);        
    }

    public Endpoint loadEndpoint(int id) {
        return (Endpoint) getHibernateTemplate().load(Endpoint.class, new Integer(id));
    }
    
    public void storeEndpointAssignment(EndpointAssignment assignment) {
        getHibernateTemplate().saveOrUpdate(assignment);        
    }
    
    public EndpointAssignment loadEndpointAssignment(int assignmentId) {    
        return (EndpointAssignment) getHibernateTemplate().load(EndpointAssignment.class, 
                new Integer(assignmentId));            
    }
    
    public void deleteEndpointAssignment(EndpointAssignment assignment) {
        getHibernateTemplate().delete(assignment);        
    }

    public void storeSetting(Setting setting, int depth) {        
        getHibernateTemplate().saveOrUpdate(setting);
        if (depth > 0 || depth == CASCADE && setting.getSettings().size() > 0) {
            Iterator children = setting.getSettings().values().iterator();
            int nextDepth = (depth == CASCADE ? CASCADE : depth - 1); 
            while (children.hasNext()) {
                Setting child = (Setting) children.next();
                storeSetting(child, nextDepth);                    
            }
        }
    }
    
    public void deleteSetting(Setting setting) {
        // PERFORMANCE: concerned about performce, cascade at database better
        // or custom script acceptable too 
        Iterator children = setting.getSettings().values().iterator();
        while (children.hasNext()) {
            deleteSetting((Setting) children.next());
        }
        getHibernateTemplate().delete(setting);                
    }
    
    
    public void storeSetting(Setting setting) {
        storeSetting(setting, 0);
    }

    public SettingSet loadSettings(int id) {
        return (SettingSet) getHibernateTemplate().load(SettingSet.class, new Integer(id));
    }
    
    /*
    public Line loadLine(User user, int position) {
        String query = "from Line where user_id = " + user.getId()
                + " and position = " + position;
        Collection line = getHibernateTemplate().find(query);
        
        return (Line) requireOneOrZero(line, query);
    }
    */

    /**
     * @throws IllegalStateException if more than one item in collection 
     * @param c
     * @param query
     * @return
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
     */
    
    public Organization loadRootOrganization() {
        return (Organization) getHibernateTemplate().load(Organization.class, new Integer(1));        
    }
}
