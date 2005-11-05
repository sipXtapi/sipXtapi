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

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.CollectionUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;


public class UserBuilder implements ApiBeanBuilder {
    
    private static final String IGNORE_LIST = "aliases, pin";
    
    private Set m_ignoreList;
    
    private CoreContext m_coreContext;
    
    public UserBuilder() {
        m_ignoreList = new HashSet();
        m_ignoreList.addAll(Arrays.asList(CollectionUtils.splitString(IGNORE_LIST)));
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public Set getIgnoreList() {
        return m_ignoreList;
    }
    
    public void toApi(Object apiObject, Object otherObject) {
        ApiBeanUtil.copyProperties(otherObject, apiObject, m_ignoreList);
        org.sipfoundry.sipxconfig.common.User other = (org.sipfoundry.sipxconfig.common.User) otherObject;
        User api = (User) apiObject;
        api.setAliases(other.getAliasesString());        
    }
    
    public void fromApi(Object apiObject, Object otherObject) {
        ApiBeanUtil.copyProperties(apiObject, otherObject, m_ignoreList);
        org.sipfoundry.sipxconfig.common.User other = (org.sipfoundry.sipxconfig.common.User) otherObject;
        User api = (User) apiObject;
        other.setAliasesString(api.getAliases());
        other.setPin(api.getPin(), m_coreContext.getAuthorizationRealm());
    }            
}  
