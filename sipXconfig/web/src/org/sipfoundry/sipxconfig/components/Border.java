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

import java.text.MessageFormat;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageValidateListener;
import org.apache.tapestry.link.StaticLink;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycle;
import org.sipfoundry.sipxconfig.site.Home;
import org.sipfoundry.sipxconfig.site.LoginPage;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.user.FirstUser;

public abstract class Border extends BaseComponent implements PageValidateListener {
    private VersionInfo m_version = new VersionInfo();

    public abstract CoreContext getCoreContext();

    public abstract SkinControl getSkin();

    /**
     * When true - page does not require login
     */
    public abstract boolean isLoginRequired();

    public abstract UserSession getUserSession();

    public abstract ApplicationLifecycle getApplicationLifecycle();

    /**
     * When true - only SUPER can see the pages, when false END_USER is accepted as well as admin
     */
    public abstract boolean isRestricted();

    public abstract IEngineService getRestartService();

    public void pageValidate(PageEvent event_) {
        if (!isLoginRequired()) {
            return;
        }

        // If there are no users, then we need to create the first user
        if (getCoreContext().getUsersCount() == 0) {
            throw new PageRedirectException(FirstUser.PAGE);
        }

        // If there are users, but no one is logged in, then force a login
        UserSession user = getUserSession();
        if (!user.isLoggedIn()) {
            redirectToLogin(getPage());
        }

        // If the logged-in user is not an admin, and this page is restricted, then
        // redirect the user to the home page since they are not worthy.
        // (We should probably use an error page instead of just tossing them home.)
        if (!user.isAdmin() && isRestricted()) {
            throw new PageRedirectException(Home.PAGE);
        }
    }

    public ILink logout(IRequestCycle cycle) {
        getApplicationLifecycle().logout();
        return new StaticLink(cycle.getAbsoluteURL("/"));
    }

    protected void redirectToLogin(IPage page) {
        LoginPage loginPage = (LoginPage) page.getRequestCycle().getPage(LoginPage.PAGE);
        throw new PageRedirectException(loginPage);
    }

    public VersionInfo getVersionInfo() {
        return m_version;
    }

    public String getHelpLink() {
        String help = getSkin().getHelpLink();
        String url = MessageFormat.format(help, (Object[]) getVersionInfo().getVersionIds());
        return url;
    }
}
