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
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.phone.PhoneContext;


public class PhoneContextDataSqueezer implements IPrimaryKeyConvertor  {
    
    private PhoneContext m_context;
    
    private Class m_class;
    
    public PhoneContextDataSqueezer(PhoneContext context, Class c) {
        if (!PrimaryKeySource.class.isAssignableFrom(c)) {
            throw new IllegalArgumentException(
                    "Adapter only accepts classes that implement PrimaryKeySource");
        }
        m_context = context;
        m_class = c;
    }
    
    public Object getPrimaryKey(Object objValue) {
        return objValue == null ? null : ((PrimaryKeySource) objValue).getPrimaryKey();
    }

    public Object getValue(Object objPrimaryKey) {
        return m_context.load(m_class, (Integer) objPrimaryKey);
    }
    
    public PhoneContext getPhoneContext() {
        return m_context;
    }
}

