/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class NewUser extends BasePage implements PageRenderListener {

    public static final String PAGE = "NewUser";

    public abstract CoreContext getCoreContext();

    public abstract User getUser();

    public abstract void setUser(User user);

    public void save(IRequestCycle cycle) {
        if (TapestryUtils.isValid(this)) {
            CoreContext core = getCoreContext();
            User user = getUser();
            core.saveUser(user);

            cycle.activate(ManageUsers.PAGE);
        }
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManageUsers.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getUser() == null) {
            setUser(new User());
        }
    }
}
