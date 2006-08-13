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
package org.sipfoundry.sipxconfig.site.permission;

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public class PermissionConverter implements IPrimaryKeyConverter {

    private PermissionManager m_manager;

    public void setManager(PermissionManager manager) {
        m_manager = manager;
    }

    public Object getPrimaryKey(Object value) {
        Permission permisison = (Permission) value;
        return permisison.getName();
    }

    public Object getValue(Object primaryKey) {
        return m_manager.getPermission((String) primaryKey);
    }
}
