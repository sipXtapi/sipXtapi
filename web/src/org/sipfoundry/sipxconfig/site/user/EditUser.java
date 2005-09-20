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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;

public abstract class EditUser extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditUser";
    
    public abstract CoreContext getCoreContext();
    
    public abstract Integer getUserId();
    public abstract void setUserId(Integer userId);
    
    public abstract User getUser();    
    public abstract void setUser(User user);    
    
    public abstract ICallback getCallback();
    public abstract void setCallback(ICallback callback);

    public void commit(IRequestCycle cycle_) {
        save();
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
    
    private void save() {
        getCoreContext().saveUser(getUser());
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getCoreContext().loadUser(getUserId());
        setUser(user);
        
        // If no callback has been given, then navigate back to Manage Users on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }
}
