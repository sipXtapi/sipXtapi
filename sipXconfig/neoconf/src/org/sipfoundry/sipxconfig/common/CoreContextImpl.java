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
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Restrictions;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class CoreContextImpl extends SipxHibernateDaoSupport implements CoreContext,
        ApplicationListener, DaoEventListener {

    public static final String CONTEXT_BEAN_NAME = "coreContextImpl";
    private static final String USER_GROUP_RESOURCE_ID = "user";
    private static final String USERNAME_PROP_NAME = "userName";
    /** nothing special about this name */
    private static final String ADMIN_GROUP_NAME = "administrators"; 
    private static final String QUERY_USER_IDS_BY_NAME_OR_ALIAS = "userIdsByNameOrAlias";
    
    private String m_authorizationRealm;

    private String m_domainName;

    private SettingDao m_settingDao;

    private Setting m_userSettingModel;

    private DaoEventPublisher m_daoEventPublisher;

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
    }

    public void deleteUser(User user) {
        getHibernateTemplate().delete(user);
    }

    public void deleteUsers(Collection userIds) {
        if (userIds.isEmpty()) {
            // no users to delete => nothing to do
            return;
        }
        List users = new ArrayList(userIds.size());
        for (Iterator i = userIds.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            User user = loadUser(id);
            users.add(user);
            m_daoEventPublisher.publishDelete(user);
        }
        getHibernateTemplate().deleteAll(users);
    }

    public User loadUser(Integer id) {
        User user = (User) getHibernateTemplate().load(User.class, id);

        return user;
    }

    public User loadUserByUserName(String userName) {
        return loadUserByUniqueProperty(USERNAME_PROP_NAME, userName);
    }

    private User loadUserByUniqueProperty(String propName, String propValue) {
        final Criterion expression = Restrictions.eq(propName, propValue);

        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(User.class).add(expression);
                return criteria.list();
            }
        };
        List users = getHibernateTemplate().executeFind(callback);
        User user = (User) DaoUtils.requireOneOrZero(users, expression.toString());

        return user;
    }

    public User loadUserByAlias(String alias) {
        return loadUserByNamedQueryAndNamedParam("userByAlias", "alias", alias);
    }

    public User loadUserByUserNameOrAlias(String userNameOrAlias) {
        return loadUserByNamedQueryAndNamedParam("userByNameOrAlias", "userNameOrAlias",
                userNameOrAlias);
    }
    
    /**
     * Check whether the user has a username or alias that collides with an existing username
     * or alias.  Check for internal collisions as well, for example, the user has an alias
     * that is the same as the username.  (Duplication within the aliases is not possible 
     * because the aliases are stored as a Set.)
     * If there is a collision, then return the bad name (username or alias).
     * Otherwise return null.
     * If there are multiple collisions, then it's arbitrary which name is returned.
     * 
     * @param user user to test
     * @return name that collides
     */
    public String checkForDuplicateNameOrAlias(User user) {
        HibernateTemplate hibernate = getHibernateTemplate();
        String result = null;
        
        // Check for duplication within the user itself
        List names = new ArrayList(user.getAliases());
        names.add(user.getUserName());
        result = checkForDuplicateString(names);
        if (result == null) {
            // Check the username.  If it is the username or alias for a different existing
            // user, then return it as a bad name.
            if (DaoUtils.checkDuplicatesByNamedQuery(hibernate, user,
                    QUERY_USER_IDS_BY_NAME_OR_ALIAS, user.getUserName(), null)) {
                result = user.getUserName();
            } else {
                // Check the aliases and return any duplicate as a bad name.
                // 
                for (Iterator iter = user.getAliases().iterator(); iter.hasNext();) {
                    String alias = (String) iter.next();
                    if (DaoUtils.checkDuplicatesByNamedQuery(hibernate, user,
                            QUERY_USER_IDS_BY_NAME_OR_ALIAS, alias, null)) {
                        result = alias;
                        break;
                    }
                }
            }
        }
        
        return result;
    }

    /**
     * Given a collection of strings, look for duplicates.  Return the first
     * duplicate found, or null if all strings are unique.
     */
    String checkForDuplicateString(Collection strings) {
        Map map = new HashMap(strings.size());
        for (Iterator iter = strings.iterator(); iter.hasNext();) {
            String str = (String) iter.next();
            if (map.containsKey(str)) {
                return str;
            }
            map.put(str, null);            
        }
        return null;
    }
    
    private User loadUserByNamedQueryAndNamedParam(String queryName, String paramName,
            Object value) {
        Collection usersColl = getHibernateTemplate().findByNamedQueryAndNamedParam(queryName,
                paramName, value);
        Set users = new HashSet(usersColl); // eliminate duplicates
        if (users.size() > 1) {
            throw new IllegalStateException(
                    "The database has more than one user matching the query " + queryName
                            + ", paramName = " + paramName + ", value = " + value);
        }
        User user = null;
        if (users.size() > 0) {
            user = (User) users.iterator().next();
        }
        return user;
    }

    /**
     * Return all users matching the userTemplate example. Empty properties of userTemplate are
     * ignored in the search. The empty string is effectively a search wildcard. Therefore if you
     * pass in userTemplate with all properties left empty, the result set contains all users. Set
     * matchUserNameOrAlias to true to look for userName matches in either the userName or aliases
     * properties. For example, if you pass in userTemplate with matchUserNameOrAlias set to true,
     * and userTemplate.userName = "102", then the search will return users with "102" as either
     * the userName or an alias. Otherwise matching is only done on the userName and aliases are
     * ignored. Strings are matched with a wildcard at the end. For example, if you pass in
     * userTemplate.firstName = "jo" then you'll get matches on users with first name = "jo" or
     * "joe" but not "flojo".
     */
    public List loadUserByTemplateUser(final User userTemplate, final boolean matchUserNameOrAlias) {
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                UserLoader loader = new UserLoader(session);
                return loader.loadUsers(userTemplate, matchUserNameOrAlias);
            }
        };
        List users = getHibernateTemplate().executeFind(callback);

        // Eliminate any duplicates in the list. See http://www.hibernate.org/117.html#A11 --
        // we can't count on the "distinct" keyword in HQL to fix this, because the query
        // sometimes uses outer joins.
        Set usersSet = new HashSet(users);
        users = new ArrayList(usersSet);
        return users;
    }

    public List loadUserByTemplateUser(User userTemplate) {
        return loadUserByTemplateUser(userTemplate, true);
    }

    public List loadUsers() {
        return getHibernateTemplate().loadAll(User.class);
    }

    public void clear() {
        Collection c = getHibernateTemplate().find("from User");
        getHibernateTemplate().deleteAll(c);
    }

    public void setDaoEventPublisher(DaoEventPublisher daoEventPublisher) {
        m_daoEventPublisher = daoEventPublisher;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask task = (InitializationTask) event;
            if (task.getTask().equals("admin-group-and-user")) {
                createAdminGroupAndInitialUserTask();
            }
        }
    }

    /**
     * Temporary hack: create a superadmin user with an empty password. We will remove this hack
     * before the product ships, and provide a bootstrap page instead so that if the product comes
     * up and there are no users, then the first user can be created.
     */
    public void createAdminGroupAndInitialUserTask() {
        Group adminGroup = m_settingDao.getGroupByName(User.GROUP_RESOURCE_ID, ADMIN_GROUP_NAME);
        if (adminGroup == null) {            
            adminGroup = new Group();
            adminGroup.setName(ADMIN_GROUP_NAME);
            adminGroup.setResource(User.GROUP_RESOURCE_ID);
            adminGroup.setDescription("Users with superadmin privileges");
        }
        Permission.SUPERADMIN.setEnabled(adminGroup, true);
        Permission.TUI_CHANGE_PIN.setEnabled(adminGroup, false);

        m_settingDao.saveGroup(adminGroup);

        // using superadmin name not to disrupt existing customers
        // can be anything
        String superadmin = "superadmin";
        User admin = loadUserByUserName(superadmin);
        if (admin == null) {
            admin = new User();
            admin.setUserName(superadmin);
            // Note: previously this hack set the pintoken to 'password', relying on another hack
            // that allowed the password and pintoken to be the same. That hack is gone so setting
            // the pintoken to 'password' would no longer work because the password would then be
            // the inverse hash of 'password' rather than 'password'.
            admin.setPintoken("");
        }

        admin.addGroup(adminGroup);
        saveUser(admin);
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public List getUserGroups() {
        return m_settingDao.getGroups(USER_GROUP_RESOURCE_ID);
    }

    public Setting getUserSettingsModel() {
        // return copy so original model stays intact
        return m_userSettingModel.copy();
    }

    public void setUserSettingModel(Setting userSettingModel) {
        m_userSettingModel = userSettingModel;
    }

    public Collection getAliasMappings() {
        List aliases = new ArrayList();
        List users = loadUsers();
        for (Iterator i = users.iterator(); i.hasNext();) {
            User user = (User) i.next();
            aliases.addAll(user.getAliasMappings(m_domainName));
        }
        return aliases;
    }

    public Collection getGroupMembers(Group group) {
        Collection users = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "userGroupMembers", "groupId", group.getId());
        return users;
    }

    public void onDelete(Object entity) {
        if (entity instanceof Group) {
            Group group = (Group) entity;
            getHibernateTemplate().update(group);
            if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
                Collection users = getGroupMembers(group);
                Iterator iusers = users.iterator();
                while (iusers.hasNext()) {
                    User user = (User) iusers.next();
                    Object[] ids = new Object[] {
                        group.getId()
                    };
                    DataCollectionUtil.removeByPrimaryKey(user.getGroups(), ids);
                    saveUser(user);
                }
            }
        }
    }

    public void onSave(Object entity_) {
    }
}
