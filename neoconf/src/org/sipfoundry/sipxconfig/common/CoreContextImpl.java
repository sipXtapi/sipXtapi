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
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class CoreContextImpl extends HibernateDaoSupport implements CoreContext {

    private static final char LIKE_WILDCARD = '%';
    
    private SipxProcessContext m_processContext;
    
    private String m_authorizationRealm;
    
    private String m_domainName;
    
    public CoreContextImpl() {
        super();
    }
    
    public String getAuthorizationRealm() {
        return m_authorizationRealm;
    }
    
    public void setAuthorizationRealm(String authorizationRealm) {
        m_authorizationRealm = authorizationRealm;
    }
    
    public String getDomainName() {
        return m_domainName;
    }

    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }

    public void saveUser(User user) {
        getHibernateTemplate().saveOrUpdate(user);
        m_processContext.generateAll();
    }
    
    public void deleteUser(User user) {
        getHibernateTemplate().delete(user);
        m_processContext.generateAll();        
    }

    public User loadUser(Integer id) {
        User user = (User) getHibernateTemplate().load(User.class, id);
        
        return user;
    }
    
    public User loadUserByDisplayId(String displayId) {
        // TODO: move query to mapping file
        String query = "from User u where u.displayId = '" + displayId + "'";
        List users = getHibernateTemplate().find(query);
        User user = (User) requireOneOrZero(users, query);
        
        return user;
    }
    
    public List loadUserByTemplateUser(User template) {
        try {
            // See a query building facade here, lot's of redundancy and areas for error. 
            // wait for more cases before refactoring...
            Criteria criteria = getHibernateTemplate().createCriteria(getSession(), User.class);            
            ArrayList ors = new ArrayList();
            if (StringUtils.isNotBlank(template.getFirstName())) {
                ors.add(Expression.ilike("firstName", template.getFirstName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getLastName())) {
                ors.add(Expression.ilike("lastName", template.getLastName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getExtension())) {
                ors.add(Expression.eq("extension", template.getExtension()));
            }
            if (StringUtils.isNotBlank(template.getDisplayId())) {
                ors.add(Expression.ilike("displayId", template.getDisplayId() + LIKE_WILDCARD));
            }
            
            if (ors.size() > 0) {
                Criterion templateExpression = (Criterion) ors.get(0);
                for (int i = 1; i < ors.size(); i++) {
                    Criterion next = (Criterion) ors.get(i);
                    templateExpression = Expression.or(templateExpression, next);
                }
                criteria.add(templateExpression);
            }
                        
            return criteria.list();
        } catch (HibernateException e) {
            throw getHibernateTemplate().convertHibernateAccessException(e);
        }
    }
    
    public List loadUsers() {
        return getHibernateTemplate().loadAll(User.class);
    }    

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
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
            // TODO: move error string construction to new UnexpectedQueryResult(?) class, enable localization
            StringBuffer error = new StringBuffer().append("read ").append(c.size())
                    .append(" and expected zero or one. query=").append(query);
            throw new IllegalStateException(error.toString());
        }        
        Iterator i = c.iterator();
        
        return (i.hasNext() ? c.iterator().next() : null);
    }    
    
    public void clear() {
        getHibernateTemplate().delete("from User");        
    }
    
    public boolean checkUserPermission(User user_, Permission p_) {
        // TODO: implementation needed
        return false;
    }
    
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }
    
    public List getUserGroups() {
        return null;
    }

    public List getUserAliases() {
        List aliases = new ArrayList();
        List users = loadUsers();
        for (Iterator i = users.iterator(); i.hasNext();) {
            User user = (User) i.next();
            aliases.addAll(user.getAliases(m_domainName));
        }
        return aliases;
    }
}
