/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.search;

import org.apache.commons.collections.Transformer;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.search.BeanAdaptor.Identity;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

/**
 * Tries to load bean from DataObjectSource if the object cannot be loaded catches exception and returns null
 * IdentityToBean
 */
public class IdentityToBean<T> implements Transformer {
    public static final Log LOG = LogFactory.getLog(IdentityToBean.class);

    private DataObjectSource<T> m_source;

    public IdentityToBean(DataObjectSource<T> source) {
        m_source = source;
    }

    public T transform(Object identity) {
        try {
            Identity<T> i = (Identity<T>) identity;
            return m_source.load(i.getBeanClass(), i.getBeanId());
        } catch (HibernateObjectRetrievalFailureException e) {
            LOG.error("Object not found: " + identity, e);
            return null;
        }
    }
}
