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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Collection;

public interface CallGroupContext {
    public static final String CONTEXT_BEAN_NAME = "callGroupContext";
    
    CallGroup loadCallGroup(Integer id);
    void storeCallGroup(CallGroup callGroup);
    void removeCallGroups(Collection ids);
}
