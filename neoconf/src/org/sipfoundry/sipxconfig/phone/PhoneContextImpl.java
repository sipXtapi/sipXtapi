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

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends HibernateDaoSupport implements BeanFactoryAware, PhoneContext {

    private BeanFactory m_beanFactory;
    
    private List m_phoneIds;
    
    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
    
    public Phone getPhone(Endpoint endpoint) {
        Phone phone = (Phone) m_beanFactory.getBean(endpoint.getPhoneId());
        if (phone != null) {
            phone.setEndpoint(endpoint);
        }
        
        return phone;
    }
    
    public Phone getPhone(int endpointId) {
        return getPhone(loadEndpoint(endpointId));
    }
        
    public List getPhoneIds() {
        return m_phoneIds;
    }

    public void setPhoneIds(List phoneIds) {
        m_phoneIds = phoneIds;
    }

    public void flush() {
        getHibernateTemplate().flush();
    }
    
    public void storeCredential(Credential credential) {
        getHibernateTemplate().saveOrUpdate(credential);                
    }
    
    public void deleteCredential(Credential credential) {
        getHibernateTemplate().delete(credential);        
    }

    public Credential loadCredential(int id) {
        return (Credential) getHibernateTemplate().load(Credential.class, new Integer(id));                
    }

    public void saveUser(User user) {
        getHibernateTemplate().saveOrUpdate(user);        
    }
    
    public void deleteUser(User user) {
        getHibernateTemplate().delete(user);        
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
        
    public void storeLine(Line line) {
        getHibernateTemplate().saveOrUpdate(line);        
    }
    
    public void deleteLine(Line line) {
        getHibernateTemplate().delete(line);        
    }

    public Line loadLine(int id) {
        return (Line) getHibernateTemplate().load(Line.class, new Integer(id));
    }

    public List loadPhoneSummaries() {        
        String endpointQuery = "from Endpoint e order by e.id";
        List endpoints = getHibernateTemplate().find(endpointQuery);
        List summaries = new ArrayList(endpoints.size());
        
        // order by same as endpoint to help juxtapositioning
        // load lines at same time, only ineffiec. for lines that
        // are on many phones, they get sent many times in db search
        // results.
        String lineQuery = "from Line l left join fetch l.user order by l.endpoint";
        List lines = getHibernateTemplate().find(lineQuery);
        
        Line line = null;
        Endpoint endpoint = null;
        PhoneSummary summary = new PhoneSummary();
        summary.setEndpointLines(new ArrayList());
        int nEndpoints = endpoints.size();
        int nlines = lines.size();
        int j = 0;
        for (int i = 0; i < nEndpoints; i++) {
            endpoint = (Endpoint) endpoints.get(i);
            line = (Line) getn(j, lines);
            while (j < nlines && line.getEndpoint().getId() == endpoint.getId()) {
                summary.getEndpointLines().add(line);                
                line = (Line) getn(j++, lines);
            }
            summary.setPhone(getPhone(endpoint));
            summaries.add(summary);
            summary = new PhoneSummary();
            summary.setEndpointLines(new ArrayList());
        }
        
        return summaries;
    }
    
    /**
     * helper routine to avoid end of list exception
     */
    private static final Object getn(int ndx, List l) {
        return (ndx < l.size() ? l.get(ndx) : null);
    }
    
    public Endpoint loadEndpoint(int id) {
        return (Endpoint) getHibernateTemplate().load(Endpoint.class, new Integer(id));
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
