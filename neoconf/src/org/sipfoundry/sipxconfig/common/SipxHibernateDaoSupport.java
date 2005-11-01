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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.hibernate.Criteria;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SipxHibernateDaoSupport extends HibernateDaoSupport {
    public static final String CONTEXT_BEAN_NAME = "sipxHibernateDaoSupport";

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }

    /**
     * Duplicate the bean and return the duplicate. If the bean is a NamedObject, then give the
     * duplicate a new, unique name. The queryName identifies a named query that returns the IDs
     * of all objects with a given name. (Return IDs rather than objects to avoid the overhead of
     * loading all the objects.) Use the query to ensure that the new name is unique.
     * 
     * @param bean bean to duplicate
     * @param queryName name of the query to be executed (define in *.hbm.xml file)
     */
    public Object duplicateBean(BeanWithId bean, String queryName) {
        BeanWithId copy = bean.duplicate();

        if (bean instanceof NamedObject) {
            // Give the new bean a unique name by prepending "copyOf" to the source
            // bean's name until we get a name that hasn't been used yet.
            HibernateTemplate template = getHibernateTemplate();
            NamedObject namedCopy = (NamedObject) copy;
            namedCopy.setName(((NamedObject) bean).getName());
            do {
                namedCopy.setName("CopyOf" + namedCopy.getName());
            } while (DaoUtils.checkDuplicatesByNamedQuery(template, copy, queryName, namedCopy
                    .getName(), null));
        }

        return copy;
    }

    public List loadBeansByPage(Class beanClass, Integer groupId, int firstRow, int pageSize,
            String orderBy, boolean orderAscending) {
        Criteria c = getByGroupCriteria(beanClass, groupId);
        c.setFirstResult(firstRow);
        c.setMaxResults(pageSize);
        if (StringUtils.isNotBlank(orderBy)) {
            Order order = orderAscending ? Order.asc(orderBy) : Order.desc(orderBy);
            c.addOrder(order);
        }
        List users = c.list();
        return users;
    }

    /**
     * Return the count of beans of type beanClass in the specified group. If groupId is null,
     * then don't filter by group, just count all the beans.
     */
    public int getBeansInGroupCount(Class beanClass, Integer groupId) {
        Criteria crit = getByGroupCriteria(beanClass, groupId);
        crit.setProjection(Projections.rowCount());
        List results = crit.list();
        if (results.size() > 1) {
            throw new RuntimeException("Querying for bean count returned multiple results!");
        }
        Integer count = (Integer) results.get(0);
        return count.intValue();
    }

    /**
     * Create and return a Criteria object for filtering beans by group membership. The class
     * passed in should extend BeanWithGroups. If groupId is null, then don't filter by group.
     */
    public Criteria getByGroupCriteria(Class klass, Integer groupId) {
        Criteria crit = getSession().createCriteria(klass);
        if (groupId != null) {
            crit.createCriteria("groups", "g");
            crit.add(Restrictions.eq("g.id", groupId));
        }
        return crit;
    }

    protected void removeAll(Class klass, Collection ids) {
        HibernateTemplate template = getHibernateTemplate();
        Collection entities = new ArrayList(ids.size());
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Object entity = template.load(klass, id);
            entities.add(entity);
        }
        template.deleteAll(entities);
    }

}
