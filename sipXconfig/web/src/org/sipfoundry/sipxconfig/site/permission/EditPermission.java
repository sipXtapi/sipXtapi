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

import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public abstract class EditPermission extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "EditPermission";

    public abstract PermissionManager getPermissionManager();

    public abstract String getPermissionName();

    public abstract void setPermissionName(String name);

    public abstract Permission getPermission();

    public abstract void setPermission(Permission permission);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public void pageBeginRender(PageEvent event_) {
        Permission permission = getPermission();
        if (null != permission) {
            return;
        }
        String id = getPermissionName();
        if (null != id) {
            PermissionManager pm = getPermissionManager();
            permission = pm.getPermission(id);
        } else {
            permission = Permission.Type.CALL.create("");
        }
        setPermission(permission);

        // If no callback was set before navigating to this page, then by
        // default, go back to the ListParkOrbits page
        if (getCallback() == null) {
            setCallback(new PageCallback(ListPermissions.PAGE));
        }
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
        pm.addCallPermission(permission);
        setPermissionName(permission.getName());
    }
}
