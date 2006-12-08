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
package org.sipfoundry.sipxconfig.site.permission;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public abstract class EditPermission extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "EditPermission";

    public abstract PermissionManager getPermissionManager();

    public abstract Object getPermissionId();

    public abstract void setPermissionId(Object id);

    public abstract Permission getPermission();

    public abstract void setPermission(Permission permission);

    public void pageBeginRender(PageEvent event_) {
        Permission permission = getPermission();
        if (null != permission) {
            return;
        }
        Object id = getPermissionId();
        if (null != id) {
            PermissionManager pm = getPermissionManager();
            permission = pm.getPermission(id);
        } else {
            permission = new Permission();
        }
        setPermission(permission);
    }

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    private boolean isValid() {
        return TapestryUtils.isValid(this);
    }

    private void saveValid() {
        PermissionManager pm = getPermissionManager();
        Permission permission = getPermission();
        if (!permission.isBuiltIn()) {
            pm.addCallPermission(permission);
            setPermissionId(permission.getPrimaryKey());
        }
    }
}
