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
import org.sipfoundry.sipxconfig.site.admin.ExtensionPoolsPage;

public abstract class NewUser extends BasePage implements PageRenderListener {

    public static final String PAGE = "NewUser";

    public abstract CoreContext getCoreContext();

    public abstract User getUser();    
    public abstract void setUser(User user);
    
    public abstract ICallback getCallback();
    public abstract void setCallback(ICallback callback);

    public void commit(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            CoreContext core = getCoreContext();
            User user = getUser();
            core.saveUser(user);
        }
    }

    public void pageBeginRender(PageEvent event) {
        if (getUser() == null) {
            setUser(new User());
        }
        
        // If the user clicks through to the Extension Pools page, clicking OK or Cancel on that
        // page should send the user back here
        ICallback callback = new PageCallback(PAGE);
        ExtensionPoolsPage poolsPage = (ExtensionPoolsPage) event.getRequestCycle().getPage(ExtensionPoolsPage.PAGE);
        poolsPage.setCallback(callback);
        
        // If no callback was set before navigating to this page, then by
        // default, go back to the ManageUsers page
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }
}
