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

import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

/**
 * Read objects by Id for Tapestry pages that need to read individual objects by their primary key. For example
 * table with select boxes. 
 */
public class HibernateDaoDataSqueezer implements IPrimaryKeyConvertor {

    private DataObjectSource m_source;

    private Class m_class;

    public HibernateDaoDataSqueezer(DataObjectSource source, Class c) {
        if (!PrimaryKeySource.class.isAssignableFrom(c)) {
            throw new IllegalArgumentException(
                    "Adapter only accepts classes that implement PrimaryKeySource");
        }
        m_source = source;
        m_class = c;
    }

    public Object getPrimaryKey(Object objValue) {
        return objValue == null ? null : ((PrimaryKeySource) objValue).getPrimaryKey();
    }

    public Object getValue(Object objPrimaryKey) {
        return m_source.load(m_class, (Integer) objPrimaryKey);
    }

    public DataObjectSource getDataObjectSource() {
        return m_source;
    }
}
