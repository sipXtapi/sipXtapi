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
    private Set m_ignoreList;
    
    public Set getIgnoreList() {
        if (m_ignoreList == null) {
            m_ignoreList = new HashSet();
        }
        
        return m_ignoreList;
    }
    
    public void toApi(Object apiObject, Object otherObject) {
        ApiBeanUtil.copyProperties(otherObject, apiObject, m_ignoreList);
    }
    
    public void fromApi(Object apiObject, Object otherObject) {
        ApiBeanUtil.copyProperties(apiObject, otherObject, m_ignoreList);
    }            
}
