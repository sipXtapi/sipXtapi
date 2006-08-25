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
package org.sipfoundry.sipxconfig.permission;

import java.util.Collection;

import org.sipfoundry.sipxconfig.setting.Setting;

public interface PermissionManager {

    public static final String CONTEXT_BEAN_NAME = "permissionManager";

    Permission getPermission(String name);

    void addCallPermission(Permission permission);

    Collection<Permission> getCallPermissions();

    void removeCallPermissions(Collection<String> permissionNames);

    Setting getPermissionModel();
}
