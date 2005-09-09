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

import java.util.Collection;
import java.util.List;

import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Utilities for Hibernate DAOs
 */
public final class DaoUtils {

    private DaoUtils() {
        // Utility class - do not instantiate
    }

    /**
     * Return true if query returns objects other than obj. Used to check for duplicates.
     * If exception is non-null, then throw the exception instead of returning true.
     * 
     * @param hibernate spring hibernate template
     * @param obj object to be checked
     * @param queryName name of the query to be executed (define in *.hbm.xml file)
     * @param value parameter for the query
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicates(HibernateTemplate hibernate, BeanWithId obj,
            String queryName, Object value, UserException exception) {
        if (value == null) {
            return false;
        }
        
        List objs = hibernate.findByNamedQueryAndNamedParam(queryName, "value", value);
        return checkDuplicates(obj, objs, exception);
    }
    
    /**
     * Return true if list contains objects other than obj. Used to check for duplicates.
     * If exception is non-null, then throw the exception instead of returning true.
     * 
     * @param obj object to be checked
     * @param objs results for query
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicates(BeanWithId obj, Collection objs, UserException exception) {
        // no match
        if (objs.size() == 0) {
            return false;       // there are no duplicates
        }

        // detect 1 match, itself
        if (!obj.isNew() && objs.size() == 1) {
            Integer found = (Integer) objs.iterator().next();
            if (found.equals(obj.getId())) {
                return false;   // there are no duplicates
            }
        }
        
        // there are duplicates
        if (exception != null) {
            throw exception;
        }
        return true;
    }
}
