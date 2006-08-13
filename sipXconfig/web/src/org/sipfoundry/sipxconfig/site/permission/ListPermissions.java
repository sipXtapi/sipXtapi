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

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.permission.PermissionManager;

public abstract class ListPermissions extends BasePage {

    public static final String PAGE = "ListPermissions";

    public abstract PermissionManager getPermissionManager();

    public abstract Collection getRowsToDelete();

    public IPage add(IRequestCycle cycle) {
        EditPermission editPage = (EditPermission) cycle.getPage(EditPermission.PAGE);
        editPage.setPermissionName(null);
        return editPage;
    }

    public IPage edit(IRequestCycle cycle, String permissionName) {
        EditPermission editPage = (EditPermission) cycle.getPage(EditPermission.PAGE);
        editPage.setPermissionName(permissionName);
        return editPage;
    }

    public void formSubmit() {
        delete();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getPermissionManager().removeCallPermissions(selectedRows);
        }
    }
}
