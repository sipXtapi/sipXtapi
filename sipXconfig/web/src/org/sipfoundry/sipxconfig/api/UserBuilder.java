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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;

public class UserBuilder extends SimpleBeanBuilder {
    
    private static final String IGNORE_LIST = "aliases, pin";
    
    private CoreContext m_coreContext;
    
    public UserBuilder() {
        getIgnoreList().addAll(Arrays.asList(SipxCollectionUtils.splitString(IGNORE_LIST)));
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void toApi(Object apiObject, Object otherObject) {
        super.toApi(apiObject, otherObject);
        org.sipfoundry.sipxconfig.common.User other = (org.sipfoundry.sipxconfig.common.User) otherObject;
        User api = (User) apiObject;
        api.setAliases(other.getAliasesString());        
    }
    
    public void fromApi(Object apiObject, Object otherObject) {
        super.fromApi(apiObject, otherObject);
        org.sipfoundry.sipxconfig.common.User other = (org.sipfoundry.sipxconfig.common.User) otherObject;
        User api = (User) apiObject;
        other.setAliasesString(api.getAliases());
        other.setPin(api.getPin(), m_coreContext.getAuthorizationRealm());
    }            
}  
