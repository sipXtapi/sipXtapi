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
package org.sipfoundry.sipxconfig.core;

import java.util.List;

import net.sf.hibernate.HibernateException;
import net.sf.hibernate.Session;

import org.springframework.orm.hibernate.SessionFactoryUtils;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Use Hibernate to r/w object to database for device related items
 */
public class DeviceDaoImpl extends HibernateDaoSupport implements DeviceDao {

    /**
     * This this be be qualified to removed altogether
     * 
     * @see org.sipfoundry.sipxconfig.core.DeviceDao#getAllDevices()
     */
    public List getAllDevices() {
        Session session = SessionFactoryUtils.getSession(getSessionFactory(), false);
        try {
            return session.find("from Device");
        } catch (HibernateException ex) {
            throw SessionFactoryUtils.convertHibernateAccessException(ex);
        }
    }

}
