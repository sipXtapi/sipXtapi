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

import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends HibernateDaoSupport implements SettingDao {
    
    private static final String RESOURCE_PARAM = "resource";
    
    public void storeValueStorage(ValueStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public ValueStorage loadValueStorage(int storageId) {
        return (ValueStorage) getHibernateTemplate().load(ValueStorage.class, new Integer(storageId));    
    }

    public void storeGroup(Group storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public Group loadGroup(int id) {
        return (Group) getHibernateTemplate().load(Group.class, new Integer(id));
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
    
    public Collection getGroups(String resource) {
        Collection tags = getHibernateTemplate().findByNamedQueryAndNamedParam("groupsByResource", 
                RESOURCE_PARAM, resource);
        return tags;
    }
}
