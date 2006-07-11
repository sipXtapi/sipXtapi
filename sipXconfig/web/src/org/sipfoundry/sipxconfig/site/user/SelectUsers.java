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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class SelectUsers extends BasePage {
    public static final String PAGE = "SelectUsers";

    public abstract CoreContext getCoreContext();

    public abstract SelectUsersCallback getCallback();

    public abstract void setCallback(SelectUsersCallback callback);

    public abstract void setTitle(String title);

    public abstract void setPrompt(String title);

    public void select(IRequestCycle cycle) {
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        SelectUsersCallback callback = getCallback();
        callback.setIds(selections.getAllSelected());
        callback.performCallback(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        SelectUsersCallback callback = getCallback();
        callback.performCallback(cycle);
    }
}
