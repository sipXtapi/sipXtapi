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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;

public abstract class LoginPage extends BasePage {

    public static final String PAGE = "LoginPage";

    public abstract String getPassword();

    public abstract void setPassword(String password);

    public abstract String getUserName();

    public abstract void setUserName(String userName);

    public abstract LoginContext getLoginContext();

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public void login(IRequestCycle cycle) {
        // always clean password property - use local variable in this function
        String password = getPassword();
        setPassword(null);

        if (!TapestryUtils.isValid(this)) {
            return;
        }

        LoginContext context = getLoginContext();
        User user = context.checkCredentials(getUserName(), password);
        if (user == null) {
            IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
            delegate.record(getMessage("message.loginError"), ValidationConstraint.CONSISTENCY);
            return;
        }

        Visit visit = (Visit) getVisit();
        visit.login(user.getId(), context.isAdmin(user));

        ICallback callback = getCallback();
        if (callback != null) {
            callback.performCallback(cycle);
        } else {
            cycle.activate(Home.PAGE);
        }
    }
}
