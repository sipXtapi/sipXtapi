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

import net.sf.hibernate.Criteria;
import net.sf.hibernate.HibernateException;
import net.sf.hibernate.expression.Criterion;
import net.sf.hibernate.expression.Expression;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends HibernateDaoSupport implements BeanFactoryAware, PhoneContext {

    private static final char LIKE_WILDCARD = '%';

    private BeanFactory m_beanFactory;
    
    private List m_phoneIds;
    
    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
    
    public Phone getPhone(Endpoint endpoint) {
        return (Phone) m_beanFactory.getBean(endpoint.getPhoneId());
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
        
    public List loadUserByTemplateUser(User template) {
        try {
            // See a query building facade here, lot's of redundancy and areas for error. 
            // wait for more cases before refactoring...
            Criteria criteria = getHibernateTemplate().createCriteria(getSession(), User.class);            
            ArrayList ors = new ArrayList();
            if (StringUtils.isNotBlank(template.getFirstName())) {
                ors.add(Expression.like("firstName", template.getFirstName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getLastName())) {
                ors.add(Expression.like("lastName", template.getLastName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getExtension())) {
                ors.add(Expression.eq("extension", template.getExtension()));
            }
            if (StringUtils.isNotBlank(template.getDisplayId())) {
                ors.add(Expression.like("displayId", template.getDisplayId() + LIKE_WILDCARD));
            }
            
            Criterion normalUsers = Expression.isNotNull("userGroupId");
            if (ors.isEmpty()) {
                criteria.add(normalUsers);
            } else {
                Criterion templateExpression = (Criterion) ors.get(0);
                for (int i = 1; i < ors.size(); i++) {
                    Criterion next = (Criterion) ors.get(i);
                    templateExpression = Expression.or(templateExpression, next);
                }
                criteria.add(Expression.and(normalUsers, templateExpression));                
            }
            
            return criteria.list();
        } catch (HibernateException e) {
            throw getHibernateTemplate().convertHibernateAccessException(e);
        }
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
        String endpointQuery = "from Endpoint e left join fetch e.lines line left join fetch line.user";
        List endpoints = getHibernateTemplate().find(endpointQuery);
        List summaries = new ArrayList(endpoints.size());
        
        // TODO: Make this a cursor usable by tapestry by reconnecting back to
        // session
        for (int i = 0; i < endpoints.size(); i++) {
            Endpoint endpoint = (Endpoint) endpoints.get(i);
            PhoneSummary summary = new PhoneSummary();
            summary.setEndpoint(endpoint);
            summary.setPhone(getPhone(endpoint));
            summaries.add(summary);
        }
        
        return summaries;
    }
    
    public Endpoint loadEndpoint(int id) {
        return (Endpoint) getHibernateTemplate().load(Endpoint.class, new Integer(id));
    }
    
    public Object load(Class c, int id) {
        return getHibernateTemplate().load(c, new Integer(id));        
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
       
    public Organization loadRootOrganization() {
        return (Organization) getHibernateTemplate().load(Organization.class, new Integer(1));        
    }
    
    /**
     * Violates
     * @param endpoint
    public void updateLinesPositions(Endpoint endpoint) {        
        shiftLinePositions(endpoint, 1000);
        storeEndpoint(endpoint);
        shiftLinePositions(endpoint, -1000);
        storeEndpoint(endpoint);
    }
    
    private void shiftLinePositions(Endpoint endpoint, int shift) {
        List lines = endpoint.getLines();
        for (int i = 0; i < lines.size(); i++) {
            Line line = (Line) lines.get(i);
            line.setPosition(line.getPosition() + shift);
        }
    }
     */
}
