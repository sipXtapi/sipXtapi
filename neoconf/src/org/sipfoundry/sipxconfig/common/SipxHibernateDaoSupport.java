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

import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SipxHibernateDaoSupport extends HibernateDaoSupport {
    public static final String CONTEXT_BEAN_NAME = "sipxHibernateDaoSupport";

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }

    /**
     * Duplicate the bean and return the duplicate.
     * If the bean is a NamedObject, then give the duplicate a new, unique name.
     * The queryName identifies a named query that returns the IDs of all objects
     * with a given name.  (Return IDs rather than objects to avoid the overhead
     * of loading all the objects.)  Use the query to ensure that the new name is
     * unique.
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
            } while (DaoUtils.checkDuplicates(template, copy, queryName, namedCopy.getName()));
        }
                
        return copy;
    }
    
}
