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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import net.sf.hibernate.Criteria;
import net.sf.hibernate.HibernateException;
import net.sf.hibernate.expression.Criterion;
import net.sf.hibernate.expression.Expression;

import org.apache.commons.lang.StringUtils;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * CoreContextImpl
 */
public class CoreContextImpl  extends HibernateDaoSupport implements CoreContext {

    private static final char LIKE_WILDCARD = '%';    

    public CoreContextImpl() {
        super();
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

    /**
     * Catch database corruption errors when more than one record exists.
     * In general fields should have unique indexes setup to protect against
     * this.  This method is created as a safe check only, there has been not
     * been any experiences of courupt data to date.
     * 
     * @param c 
     * @param query
     * 
     * @return first item from the collection
     * @throws IllegalStateException if more than one item in collection. In general
     */
    public static Object requireOneOrZero(Collection c, String query) {
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
}
