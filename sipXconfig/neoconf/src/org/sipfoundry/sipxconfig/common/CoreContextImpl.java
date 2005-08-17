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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.hibernate.Criteria;
import org.hibernate.HibernateException;
import org.hibernate.cfg.Configuration;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.hibernate.event.SaveOrUpdateEvent;
import org.hibernate.event.SaveOrUpdateEventListener;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class CoreContextImpl extends HibernateDaoSupport 
        implements CoreContext, ApplicationListener {

    static final String USER_GROUP_RESOURCE_ID = "user";

    private static final char LIKE_WILDCARD = '%';
    private static final String USERNAME_PROP_NAME = "userName";
    private static final String EXTENSION_PROP_NAME = "extension";
            
    private SipxProcessContext m_processContext;
    
    private String m_authorizationRealm;
    
    private String m_domainName;
    
    private SettingDao m_settingDao;
    
    private Setting m_userSettingModel;
    
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
    
    public void deleteUsers(Collection users) {
        if (CollectionUtils.safeSize(users) == 0) {
            return;     // no users to delete => nothing to do
        }
        for (Iterator iter = users.iterator(); iter.hasNext();) {
            User user = (User) iter.next();
            getHibernateTemplate().delete(user);            
        }
        m_processContext.generateAll();
    }

    public User loadUser(Integer id) {
        User user = (User) getHibernateTemplate().load(User.class, id);
        
        return user;
    }
    
    public User loadUserByUserName(String userName) {
        return loadUserByUniqueProperty(USERNAME_PROP_NAME, userName);
    }

    public User loadUserByExtension(String extension) {
        return loadUserByUniqueProperty(EXTENSION_PROP_NAME, extension);
    }
    
    private User loadUserByUniqueProperty(String propName, String propValue) {
        // TODO: move query to mapping file
        String query = "from User u where u." + propName + " = '" + propValue + "'";
        List users = getHibernateTemplate().find(query);
        User user = (User) requireOneOrZero(users, query);
        
        return user;        
    }
    
    public List loadUserByTemplateUser(User template) {
        try {
            // See a query building facade here, lot's of redundancy and areas for error. 
            // wait for more cases before refactoring...
            
            Criteria criteria = getSession().createCriteria(User.class);            
            
            ArrayList ors = new ArrayList();
            if (StringUtils.isNotBlank(template.getFirstName())) {
                ors.add(Restrictions.ilike("firstName", template.getFirstName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getLastName())) {
                ors.add(Restrictions.ilike("lastName", template.getLastName() + LIKE_WILDCARD));
            }
            if (StringUtils.isNotBlank(template.getExtension())) {
                ors.add(Restrictions.eq(EXTENSION_PROP_NAME, template.getExtension()));
            }
            if (StringUtils.isNotBlank(template.getUserName())) {
                ors.add(Restrictions.ilike(USERNAME_PROP_NAME, template.getUserName() + LIKE_WILDCARD));
            }
            
            if (ors.size() > 0) {
                Criterion templateExpression = (Criterion) ors.get(0);
                for (int i = 1; i < ors.size(); i++) {
                    Criterion next = (Criterion) ors.get(i);
                    templateExpression = Restrictions.or(templateExpression, next);
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
        Collection c = getHibernateTemplate().find("from User");
        getHibernateTemplate().deleteAll(c);        
    }
    
    public boolean checkUserPermission(User user_, Permission p_) {
        // TODO: implementation needed
        return false;
    }
    
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }
    
    public SipxProcessContext getProcessContext() {
        return m_processContext;
    }
    

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent) {
            Configuration cfg = new Configuration();
            cfg.getSessionEventListenerConfig()
                    .setSaveOrUpdateEventListener(new PermissionReplicationTrigger(m_processContext));            
        } else if (event instanceof InitializationTask) {
            InitializationTask task = (InitializationTask) event;
            if (task.getTask().equals("default-user-group")) {
                m_settingDao.createRootGroup(USER_GROUP_RESOURCE_ID);
            }
        }
    }       
        
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }    
    
    public Group loadRootUserGroup() {
        return m_settingDao.loadRootGroup(USER_GROUP_RESOURCE_ID);
    }
    
    public List getUserGroups() {
        return m_settingDao.getGroups(USER_GROUP_RESOURCE_ID);
    }

    public List getUserGroupsWithoutRoot() {
        return m_settingDao.getGroupsWithoutRoot(USER_GROUP_RESOURCE_ID);
    }

    public Setting getUserSettingsModel() {
        // return copy so original model stays intact
        return m_userSettingModel.copy();
    }
    
    public void setUserSettingModel(Setting userSettingModel) {
        m_userSettingModel = userSettingModel;
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
    
    /**
     * Watch for user permissions changes, on change, 
     */
    static class PermissionReplicationTrigger implements SaveOrUpdateEventListener {
        
        private SipxProcessContext m_sipx;
        
        PermissionReplicationTrigger(SipxProcessContext sipx) {
            m_sipx = sipx;
        }
    
        public Serializable onSaveOrUpdate(SaveOrUpdateEvent event) {
            if (event.getClass().equals(Group.class)) {
                Group g = (Group) event.getEntity();
                if (CoreContextImpl.USER_GROUP_RESOURCE_ID.equals(g.getResource())) {
                    m_sipx.generate(DataSet.PERMISSION);
                }            
            }
            
            return null;
        }
    }
}
