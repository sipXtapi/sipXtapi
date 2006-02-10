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
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.LoginPage;

public abstract class FirstUser extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "FirstUser";

    public abstract CoreContext getCoreContext();
    
    public abstract String getPin();
    public abstract void setPin(String pin);

    public void pageBeginRender(PageEvent event) {
        // This page runs only when there are no users, and the first user
        // needs to be created.  If a user exists, then bail out to the login page.
        // After we create the user, we'll land here and go to login.
        if (getCoreContext().getUsersCount() != 0) {
            LoginPage loginPage = (LoginPage) event.getRequestCycle().getPage(LoginPage.PAGE);
            throw new PageRedirectException(loginPage);
        }
    }

    public void commit(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {            
            // Create the superadmin user
            getCoreContext().createAdminGroupAndInitialUser(getPin());
        }
    }
}
