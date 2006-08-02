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
package org.sipfoundry.sipxconfig.api;

import java.util.HashSet;
import java.util.Set;

/**
 * Copies all getters from one bean into all the available, matching
 * setters of another bean.
 */
public class SimpleBeanBuilder implements ApiBeanBuilder {    
    public static final String ID_PROP = "id";
    
    private Set m_customFields;
    
    public Set getCustomFields() {
        if (m_customFields == null) {
            m_customFields = new HashSet();
        }
        
        return m_customFields;
    }
    
    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        ApiBeanUtil.copyProperties(apiObject, myObject, properties, m_customFields);
    }
    
    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        ApiBeanUtil.copyProperties(myObject, apiObject, properties, m_customFields);
    }            
}
