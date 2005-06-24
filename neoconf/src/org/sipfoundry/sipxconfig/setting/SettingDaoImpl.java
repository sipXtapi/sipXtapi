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
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends HibernateDaoSupport implements SettingDao {
    
    private static final String RESOURCE_PARAM = "resource";
    
    public void storeValueStorage(ValueStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public ValueStorage loadValueStorage(Integer storageId) {
        return (ValueStorage) getHibernateTemplate().load(ValueStorage.class, storageId);    
    }

    public void storeGroup(Group group) {
        checkDuplicates(group);
        getHibernateTemplate().saveOrUpdate(group);                        
    }
    
    void checkDuplicates(Group group) {
        String[] params = new String[] {
            RESOURCE_PARAM,
            "name"
        };
        Object[] values = new Object[] {
                group.getResource(),
                group.getName()
        };
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam("groupIdsWithNameAndResource", params, values);
        DaoUtils.checkDuplicates(group, objs, new DuplicateGroupException(group.getName()));        
    }
    
    private class DuplicateGroupException extends UserException {
        private static final String ERROR = "A group with name: {0} already exists.";

        public DuplicateGroupException(String name) {
            super(ERROR, name);
        }
    }
    
    public Group loadGroup(Integer id) {
        return (Group) getHibernateTemplate().load(Group.class, id);
    }

    public Group loadRootGroup(String resource) {
        Group tag;
        String query = "rootGroupByResource";
        Collection tags = getHibernateTemplate().findByNamedQueryAndNamedParam(query, 
                RESOURCE_PARAM, resource);
        if (tags.size() == 0) {
            tag = new Group();
            tag.setName("Default");
            tag.setResource(resource);      
            storeGroup(tag);
        } else {
            tag = (Group) CoreContextImpl.requireOneOrZero(tags, query);
        }
        
        return tag;
    }
    
    public List getGroups(String resource) {
        List groups = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResource", 
                RESOURCE_PARAM, resource);
        return groups;
    }
    
    public List getGroupsWithoutRoot(String resource) {
        List groups = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResourceWithoutRoot", 
                RESOURCE_PARAM, resource);
        return groups;        
    }

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }
}
