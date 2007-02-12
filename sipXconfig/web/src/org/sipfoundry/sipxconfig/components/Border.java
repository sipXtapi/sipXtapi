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
import org.apache.tapestry.IExternalPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.callback.ExternalCallback;
import org.apache.tapestry.callback.ICallback;
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

    public void pageValidate(PageEvent event) {
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
            redirectToLogin(getPage(), event.getRequestCycle());
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

    protected void redirectToLogin(IPage page, IRequestCycle cycle) {
        LoginPage login = (LoginPage) page.getRequestCycle().getPage(LoginPage.PAGE);
        conditionallySetLoginCallback(login, page, cycle);        
        throw new PageRedirectException(login);
    }
    
    private void conditionallySetLoginCallback(LoginPage login, IPage page, IRequestCycle cycle) {
        // External pages can safely be redirected to after successful login
        if (page instanceof IExternalPage) {
            //
            // If page service is calling page, you're probably logged out and user could be
            // looking at a very different page then what gets interpretted from this callback
            // because all session data is lost. 
            //
            // If your page can safely handle this, consider refactoring border component to accept a
            // component parameter to circumvent this constraint.  Warning: this means your page 
            // either doesn't use session variabled OR can safely recover when session is lost
            //
            String serviceName = cycle.getService().getName(); 
            if (!"page".equals(serviceName)) {                
                ICallback callback = new ExternalCallback((IExternalPage) page, cycle.getListenerParameters());
                login.setCallback(callback);            
            }
        }        
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