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

import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use hibernate to perform database operations
 */
public class SettingDaoImpl extends HibernateDaoSupport implements SettingDao {
    
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
        // Represents the single meta storage id that holds the defaults
        // for all settings groups.
        String query = "from Tag f where f.label = 'Default' and f.resource = '" + resource + "'";
        List tags = getHibernateTemplate().find(query);
        if (tags.size() == 0) {
            tag = new Tag();
            tag.setResource(resource);      
            storeTag(tag);
        } else {
            tag = (Tag) CoreContextImpl.requireOneOrZero(tags, query);
        }
        
        return tag;
    }
}
