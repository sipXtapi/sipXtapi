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

import net.sf.hibernate.ObjectNotFoundException;

import org.springframework.orm.hibernate.HibernateObjectRetrievalFailureException;
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

    public void storeMetaStorage(MetaStorage storage) {
        getHibernateTemplate().saveOrUpdate(storage);                        
    }
    
    public MetaStorage loadRootMetaStorage() {
        // Represents the single meta storage id that holds the defaults
        // for all settings groups.
        MetaStorage meta = null;
        try {
            meta = (MetaStorage) getHibernateTemplate().load(MetaStorage.class, new Integer(1));
        } catch (HibernateObjectRetrievalFailureException e) {
            Throwable cause = e.getCause();
            // make sure only reason we create new object is if it's not 
            // created yet.  Spring wraps 4 different exceptio to one.
            if (cause != null && cause instanceof ObjectNotFoundException) {
                meta = new MetaStorage();
            } else {
                throw e;
            }
        }
        
        return meta;
    }
}
