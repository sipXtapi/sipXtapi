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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

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
    
    public User loadUserByDisplayId(String displayId) {
        String query = "from User u where u.displayId = '" + displayId + "'";
        List users = getHibernateTemplate().find(query);
        
        return (User) requireOneOrZero(users, query);
    }
    
    public void storeEndpoint(Endpoint endpoint) {
        getHibernateTemplate().saveOrUpdate(endpoint);        
    }
    
    public void deleteEndpoint(Endpoint endpoint) {
        getHibernateTemplate().delete(endpoint);        
    }
        
    public List loadPhoneSummaries(PhoneSummaryFactory factory) {        
        String endpointQuery = "from Endpoint e order by e.id";
        List endpoints = getHibernateTemplate().find(endpointQuery);
        List summaries = new ArrayList(endpoints.size());
        
        // order by same as endpoint to help juxtapositioning 
        String assignmentQuery = "from EndpointAssignment ea order by ea.endpoint";
        List assignments = getHibernateTemplate().find(assignmentQuery);
        
        Iterator iendpoints = endpoints.iterator();
        EndpointAssignment assignment = null;
        PhoneContext context = factory.getPhoneContext();
        for (int j = 0; iendpoints.hasNext();) {
            PhoneSummary summary = factory.createPhoneSummary();
            Endpoint endpoint = (Endpoint) iendpoints.next();
            summary.setPhone(context.getPhone(endpoint));
            // juxtapostion endpoint assignments
            if (j < assignments.size()) {
                assignment = (EndpointAssignment) assignments.get(j);
                if (assignment.getEndpoint().equals(endpoint)) {
                    summary.setAssignment(assignment);
                    j++;
                }
            }
            
            summaries.add(summary);
        }
        
        return summaries;
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
    
    /**
     * Catch database curruption errors when more than one record exists.
     * In general fields should have unique indexes setup to protect against
     * this.  This method is created as a safe check only, there has been not
     * been any experiences of courupt data to date.
     * 
     * @throws IllegalStateException if more than one item in collection. In general
     * 
     * @param c
     * @param query
     * @return
     */
    private Object requireOneOrZero(Collection c, String query) {
        if (c.size() > 2) {
            // DatabaseCorruptionExection ?
            StringBuffer error = new StringBuffer().append("read ").append(c.size())
                    .append(" and expected zero or one. query=").append(query);
            throw new IllegalStateException(error.toString());
        }        
        Iterator i = c.iterator();
        
        return (i.hasNext() ? c.iterator().next() : null);
    }    
    
    
    public void deleteSetting(Setting setting) {
        // PERFORMANCE: concerned about performance, cascade at database better
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
    
    public Organization loadRootOrganization() {
        return (Organization) getHibernateTemplate().load(Organization.class, new Integer(1));        
    }
}
