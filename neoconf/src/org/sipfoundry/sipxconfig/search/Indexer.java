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
package org.sipfoundry.sipxconfig.search;

import java.io.Serializable;

import org.hibernate.type.Type;

public interface Indexer {
    public String DEFAULT_FIELD = "all";

    void indexBean(Object bean, Serializable id, Object[] state, String[] fieldNames, Type[] types);

    void removeBean(Object bean, Serializable id);

    void open();

    void close();
}
