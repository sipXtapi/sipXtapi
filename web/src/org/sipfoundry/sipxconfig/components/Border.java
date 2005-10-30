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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageValidateListener;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.site.Home;
import org.sipfoundry.sipxconfig.site.LoginPage;
import org.sipfoundry.sipxconfig.site.Visit;
import org.sipfoundry.sipxconfig.site.user.FirstUser;

public abstract class Border extends BaseComponent implements PageValidateListener {
    public abstract CoreContext getCoreContext();
    
    /**
     * When true - page does not require login
     */
    public abstract boolean isLoginRequired();
    
    /**
     * When true - only SUPER can see the pages, when false END_USER is accepted as well as admin
     */
    public abstract boolean isRestricted();

    public void pageValidate(PageEvent event_) {
        if (!isLoginRequired()) {
            return;
        }

        // If there are no users, then we need to create the first user
        if (getCoreContext().getUsersCount() == 0) {
            throw new PageRedirectException(FirstUser.PAGE);
        }            

        // If there are users, but no one is logged in, then force a login
        IPage page = getPage();
        Visit visit = getVisit();        
        if (visit.getUserId() == null) {
            redirectToLogin(page);
        }
        
        // If the logged-in user is not an admin, and this page is restricted, then 
        // redirect the user to the home page since they are not worthy.
        // (We should probably use an error page instead of just tossing them home.)
        if (!visit.isAdmin() && isRestricted()) {
            throw new PageRedirectException(Home.PAGE);
        }
    }

    public void logout(IRequestCycle cycle) {
        Visit visit = getVisit();
        visit.logout(cycle);
    }
    
    protected void redirectToLogin(IPage page) {
        ICallback callback = new PageCallback(page.getPageName());
        LoginPage loginPage = (LoginPage) page.getRequestCycle().getPage(LoginPage.PAGE);
        loginPage.setCallback(callback);
        throw new PageRedirectException(loginPage);
    }
    
    protected Visit getVisit() {
        return (Visit) getPage().getVisit();
    }
}
