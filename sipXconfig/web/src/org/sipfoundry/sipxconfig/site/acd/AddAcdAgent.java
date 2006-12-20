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
package org.sipfoundry.sipxconfig.site.acd;

import java.io.Serializable;

import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.site.user.UserTable;

public abstract class AddAcdAgent extends PageWithCallback {
    public static final String PAGE = "AddAcdAgent";

    public abstract AcdContext getAcdContext();

    public abstract CoreContext getCoreContext();

    public abstract Serializable getAcdQueueId();

    public abstract void setAcdQueueId(Serializable id);

    public void select(IRequestCycle cycle) {
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        getAcdContext().addUsersToQueue(getAcdQueueId(), selections.getAllSelected());
        getCallback().performCallback(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
