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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.sipfoundry.sipxconfig.admin.callgroup.UserRing;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;

public abstract class UserRingTable extends BaseComponent {
    public abstract CoreContext getCoreContext();

    public abstract Collection getRowsToDelete();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();

    public abstract boolean getAddRow();

    public IPrimaryKeyConvertor getIdConverter() {
        CoreContext context = getCoreContext();
        return new ObjectSourceDataSqueezer(context, UserRing.class);
    }
}
