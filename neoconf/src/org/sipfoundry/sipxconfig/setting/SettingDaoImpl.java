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
}
