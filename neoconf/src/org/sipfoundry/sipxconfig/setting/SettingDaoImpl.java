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

    public void storeTag(Tag storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public Tag loadTag(int id) {
        return (Tag) getHibernateTemplate().load(Tag.class, new Integer(id));
    }

    public Tag loadRootTag(String resource) {
        Tag tag;
        String query = "rootTag";
        Collection tags = getHibernateTemplate().findByNamedQueryAndNamedParam(query, RESOURCE_PARAM, resource);
        if (tags.size() == 0) {
            tag = new Tag();
            tag.setResource(resource);      
            storeTag(tag);
        } else {
            tag = (Tag) CoreContextImpl.requireOneOrZero(tags, query);
        }
        
        return tag;
    }
    
    public Collection getTags(String resource) {
        Collection tags = getHibernateTemplate().findByNamedQueryAndNamedParam("tags", RESOURCE_PARAM, resource);
        return tags;
    }
}
