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
public class CoreDaoImpl extends HibernateDaoSupport implements CoreDao {

    /**
     * Find a single instance of an object by id
     */
    public Object findById(Class c, int id) {
        String objectTable = stripPackage(c.getName());
        Session session = SessionFactoryUtils.getSession(getSessionFactory(), false);
        try {
            return single(session.find("from " + objectTable + " where id = "
                + id));
        } catch (HibernateException ex) {
            throw SessionFactoryUtils.convertHibernateAccessException(ex);
        }
    }

    /**
     * Find a single instance of an object by id
     */
    public Object requireById(Class c, int id) {
        Object o = findById(c, id);
        if (o == null) {
            HibernateException e = new HibernateException("Required object of type " 
                    + c.getName() + " and id = " + id + " cannot be found");
            throw SessionFactoryUtils.convertHibernateAccessException(e);
        }

        return o;
    }
        
    String stripPackage(String className) {
        int dot = className.lastIndexOf('.');

        return dot == -1 ? className : className.substring(dot);
    }

    Object single(List l) {
        if (l.size() > 1) {
            HibernateException e = new HibernateException(l.size() 
                + " duplicate objects when expecting only one");
            throw SessionFactoryUtils.convertHibernateAccessException(e);
        }

        return (l.size() == 1 ? l.get(0) : null);
    }
}
