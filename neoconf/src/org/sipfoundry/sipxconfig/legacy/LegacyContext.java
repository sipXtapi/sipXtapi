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
package org.sipfoundry.sipxconfig.legacy;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.User;

public interface LegacyContext {
    public static final String CONTEXT_BEAN_NAME = "legacyContext";

    public abstract UserConfigSet getConfigSetForUser(User user);
    
    public abstract void updateConfigSet(ConfigSet cfg);
    
    public abstract boolean checkUserPermission(User user, Permission permission);
    
    public abstract void triggerCredentialGeneration();    
}
