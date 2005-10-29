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
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;

public abstract class EditUser extends PageWithCallback implements PageRenderListener {

    public static final String PAGE = "EditUser";

    public abstract CoreManager getCoreContext();

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public void commit(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            getCoreContext().saveUser(getUser());
        }
    }

    public void editCallForwarding(IRequestCycle cycle) {
        UserCallForwarding page = (UserCallForwarding) cycle.getPage(UserCallForwarding.PAGE);
        Integer userId = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        page.setUserId(userId);

        // When we navigate to the UserCallForwarding page, clicking OK or Cancel on that
        // page should send the user back here
        ICallback callback = new PageCallback(PAGE);
        page.setCallback(callback);

        cycle.activate(page);
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);
        }

        // If no callback has been given, then navigate back to Manage Users on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }
}
