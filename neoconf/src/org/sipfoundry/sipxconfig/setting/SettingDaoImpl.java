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

    public void storeFolder(Folder storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public Folder loadFolder(int id) {
        return (Folder) getHibernateTemplate().load(Folder.class, new Integer(id));
    }
    
    public Folder loadRootFolder(String resource) {
        Folder folder;
        // Represents the single meta storage id that holds the defaults
        // for all settings groups.
        String query = "from Folder f where f.parent is null and f.resource = '" + resource + "'";
        List folders = getHibernateTemplate().find(query);
        if (folders.size() == 0) {
            folder = new Folder();
            folder.setResource(resource);      
            storeFolder(folder);
        } else {
            folder = (Folder) CoreContextImpl.requireOneOrZero(folders, query);
        }
        
        return folder;
    }
}
