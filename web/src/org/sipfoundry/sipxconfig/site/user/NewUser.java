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


public abstract class NewUser extends BasePage implements PageRenderListener {
    
    public abstract CoreContext getCoreContext();
    
    public abstract User getUser();
    
    public abstract void setUser(User user);
    
    public void save(IRequestCycle cycle) {
        getCoreContext().saveUser(getUser());
        cycle.activate(ManageUsers.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManageUsers.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        setUser(new User());
    }
}
