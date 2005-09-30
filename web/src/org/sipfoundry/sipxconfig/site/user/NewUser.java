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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.admin.ExtensionPoolsPage;

public abstract class NewUser extends PageWithCallback implements PageRenderListener {

    public static final String PAGE = "NewUser";

    public abstract CoreContext getCoreContext();
    
    public abstract Integer getUserId();
    public abstract void setUserId(Integer userId);

    public abstract User getUser();    
    public abstract void setUser(User user);

    public void commit(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            // Save the user
            CoreContext core = getCoreContext();
            User user = getUser();
            core.saveUser(user);
            
            // On saving the user, it is given an ID.  Remember that ID so that if this
            // page gets an Apply rather than OK, then we know that we have already 
            // saved the user.
            setUserId(user.getId());
        }
    }

    /** Wrapper for callback so we can clear state on returning from this page */
    private class CallbackWrapper implements ICallback {
        private ICallback m_callback;
        public CallbackWrapper(ICallback callback) {
            m_callback = callback;
        }
        public void performCallback(IRequestCycle cycle) {
            setUserId(null);
            m_callback.performCallback(cycle);
        }        
    }

    public void pageBeginRender(PageEvent event) {
        if (getUserId() == null) {
            setUser(new User());            
        } else {
            User user = getCoreContext().loadUser(getUserId());
            setUser(user);            
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
                
        // Wrap the callback so we can clear state when returning from this page
        if (!(getCallback() instanceof CallbackWrapper)) {  // wrap it only once
            setCallback(new CallbackWrapper(getCallback()));
        }
        
    }
}
