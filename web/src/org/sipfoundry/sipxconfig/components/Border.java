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
import org.sipfoundry.sipxconfig.site.Home;
import org.sipfoundry.sipxconfig.site.LoginPage;
import org.sipfoundry.sipxconfig.site.Visit;

public abstract class Border extends BaseComponent implements PageValidateListener {
    
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

        IPage page = getPage();
        Visit visit = getVisit();        
        if (visit.getUserId() == null) {
            redirectToLogin(page);
        }
        if (!visit.isAdmin() && isRestricted()) {
            // probably we should have a different page for that
            throw new PageRedirectException(Home.PAGE);
        }
    }

    public void logout(IRequestCycle cycle) {
        Visit visit = getVisit();
        visit.logout();
        cycle.activate(Home.PAGE);        
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
