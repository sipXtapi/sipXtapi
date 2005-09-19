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

import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.StringUtils;
import org.hibernate.Criteria;
import org.hibernate.Query;
import org.hibernate.Session;
import org.hibernate.criterion.Criterion;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.springframework.orm.hibernate3.HibernateCallback;
import org.springframework.orm.hibernate3.HibernateTemplate;

/**
 * Utilities for Hibernate DAOs
 */
public final class DaoUtils {
    private static final String ID_PROPERTY_NAME = "id";
    private static final String DOT = ".";
    private static final String SPACE = " ";
    
    private DaoUtils() {
        // Utility class - do not instantiate
    }

    /**
     * Return true if query returns objects other than obj. Used to check for duplicates.
     * The query returns the ID strings of all objects for which the specified property
     * has the specified value.
     * If exception is non-null, then throw the exception instead of returning true.
     * 
     * @param hibernate spring hibernate template
     * @param obj object to be checked
     * @param propName name of the property to be checked
     * @param propValue property value
     * @param exception exception to throw if query returns other object than passed in the query
     */
    public static boolean checkDuplicates(final HibernateTemplate hibernate, final BeanWithId obj,
            final String propName, UserException exception) {
        Object propValue = getProperty_(obj, propName);
        if (propValue == null) {
            return false;
        }
        final Criterion expression = Restrictions.eq(propName, propValue);
        HibernateCallback callback = new HibernateCallback() {
            public Object doInHibernate(Session session) {
                Criteria criteria = session.createCriteria(obj.getClass()).add(expression)
                        .setProjection(Projections.property(ID_PROPERTY_NAME));
                return criteria.list();
            }
        };
        List objs = hibernate.executeFind(callback);
        return checkDuplicates(obj, objs, exception);
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
    public static boolean checkDuplicatesByNamedQuery(HibernateTemplate hibernate, BeanWithId obj,
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

    /**
     * Catch database corruption errors where more than one record exists. In general fields
     * should have unique indexes set up to protect against this. This method is created as a safe
     * check only, there have been not been any experiences of corrupt data to date.
     * 
     * @param c
     * @param query
     * 
     * @return first item from the collection
     * @throws IllegalStateException if more than one item in collection.
     */
    public static Object requireOneOrZero(Collection c, String query) {
        if (c.size() > 1) {
            // DatabaseCorruptionException ?
            // TODO: move error string construction to new UnexpectedQueryResult(?) class, enable
            // localization
            StringBuffer error = new StringBuffer().append("read ").append(c.size()).append(
                    " and expected zero or one. query=").append(query);
            throw new IllegalStateException(error.toString());
        }
        Iterator i = c.iterator();

        return (i.hasNext() ? c.iterator().next() : null);
    }
    
    // Put an underscore at the end of the method name to suppress a bogus
    // warning from Checkstyle about this method being unused.
    private static Object getProperty_(Object obj, String propName) {
        Object propValue = null;
        try {
            propValue = BeanUtils.getProperty(obj, propName);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e);
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
        return propValue;
    }
    
    // TODO: Consider rewriting this method to use the Criteria API rather than HQL.
    // Since the method has gotten more complex, there is now too much clever string manipulation.
    public static List loadByPage(Session session, String query, String objName, int firstRow,
            int pageSize, String orderBy, boolean orderAscending) {
        StringBuffer buf = new StringBuffer(query);       
        addOrderByClause(buf, objName, orderBy, orderAscending);
        Query q = session.createQuery(buf.toString());
        q.setFirstResult(firstRow);
        q.setMaxResults(pageSize);
        List items = q.list();
        
        return items;
    }

    private static void addOrderByClause(StringBuffer buf, String objName, String orderBy,
            boolean orderAscending) {
        if (StringUtils.isEmpty(orderBy)) {
            return;     // nothing to add
        }

        // HACK: To order by userName for the User class, we must join with the user_name table.
        // TODO: Fix the class-specific hackery in this method, probably by switching to Criteria from HQL.
        final String userNameObjectAlias = "un";
        if (orderBy.equals(User.USER_NAME_PROPERTY)) {
            // add, for example, "join u.userNameObject un"
            buf.append(" join " + objName + DOT + User.USER_NAME_OBJECT_PROPERTY + SPACE + userNameObjectAlias);
        }
        buf.append(" order by ");
        
        // Add order by constraints
        String orderDirection = orderAscending ? " asc" : " desc";
        if (orderBy.equals(User.USER_NAME_PROPERTY)) {
            buf.append(userNameObjectAlias + DOT + "name");
        } else {
            if (objName != null) {
                buf.append(objName + DOT);
            }
            buf.append(orderBy);
        }
        buf.append(orderDirection);
    }
    
}
