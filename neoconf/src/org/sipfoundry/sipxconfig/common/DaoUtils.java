/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import org.springframework.orm.hibernate.HibernateTemplate;

/**
 * Utilities for Hibernate DAOs
 */
public final class DaoUtils {

    private DaoUtils() {
        // Utility class - do not instantiate
    }

    /**
     * Throws exception if query returns other objects than obj. Used to check for duplicates.
     * 
     * @param hibernate spring hibernate template
     * @param obj object to be checked
     * @param queryName name of the query to be executed (define in *.hbm.xml file)
     * @param value parameter for the query
     * @param exception throws if query returns other object than passed in the query
     */
    public static void checkDuplicates(HibernateTemplate hibernate, BeanWithId obj,
            String queryName, Object value, UserException exception) {
        if (value == null) {
            return;
        }
        
        List objs = hibernate.findByNamedQueryAndNamedParam(queryName, "value", value);

        // no match
        if (objs.size() == 0) {
            return;
        }

        // detect 1 match, itself
        if (!obj.isNew() && objs.size() == 1) {
            Integer found = (Integer) objs.get(0);
            if (found.equals(obj.getId())) {
                return;
            }
        }
        throw exception;
    }
}
