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
package org.sipfoundry.sipxconfig.components;

import net.sf.hibernate.FlushMode;
import net.sf.hibernate.HibernateException;
import net.sf.hibernate.Session;
import net.sf.hibernate.SessionFactory;

import org.springframework.dao.CleanupFailureDataAccessException;
import org.springframework.orm.hibernate.SessionFactoryUtils;
import org.springframework.orm.hibernate.support.OpenSessionInViewFilter;

/**
 * Avoids error:
 *   Write operations are not allowed in read-only mode (FlushMode.NEVER) - turn your Session into
 *   FlushMode.AUTO respectively remove 'readOnly' marker from transaction definition
 */
public class FlushingSpringSessionInViewFilter extends OpenSessionInViewFilter {

    protected Session getSession(SessionFactory sessionFactory) {

        Session session = SessionFactoryUtils.getSession(sessionFactory, true);
        session.setFlushMode(FlushMode.AUTO);

        return session;
    }

    protected void closeSession(Session session, SessionFactory sessionFactory) {

        if (session != null && session.isOpen() && session.isConnected()) {
            try {
                session.flush();
            } catch (HibernateException e) {
                throw new CleanupFailureDataAccessException(
                        "Failed to flush session before close: " + e.getMessage(), e);
            }
        }
        super.closeSession(session, sessionFactory);
    }
}
