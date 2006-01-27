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

import org.apache.commons.lang.RandomStringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.admin.ExtensionPoolsPage;

public abstract class NewUser extends PageWithCallback implements PageRenderListener {

    public static final String PAGE = "NewUser";
    private static final int SIP_PASSWORD_LEN = 8;

    public abstract CoreContext getCoreContext();

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract boolean isStay();

    public abstract boolean isAppliedPressed();

    public void onCommit(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        // Save the user
        CoreContext core = getCoreContext();
        User user = getUser();
        core.saveUser(user);

        // On saving the user, transfer control to edituser page.
        if (isAppliedPressed()) {
            EditUser edit = (EditUser) cycle.getPage(EditUser.PAGE);
            edit.setUserId(user.getId());
            cycle.activate(edit);
        }
    }

    public void extensionPools(IRequestCycle cycle) {
        ExtensionPoolsPage poolsPage = (ExtensionPoolsPage) cycle
                .getPage(ExtensionPoolsPage.PAGE);
        poolsPage.activatePageWithCallback(PAGE, cycle);
    }

    public void activatePageWithCallback(String returnPageName, IRequestCycle cycle) {
        super.activatePageWithCallback(returnPageName, cycle);
        setCallback(new OptionalStay(getCallback()));
    }

    class OptionalStay implements ICallback {
        private ICallback m_delegate;

        OptionalStay(ICallback delegate) {
            m_delegate = delegate;
        }

        public void performCallback(IRequestCycle cycle) {
            if (isStay()) {
                setUser(null);
                cycle.activate(PAGE);
            } else if (m_delegate != null) {
                m_delegate.performCallback(cycle);
            }
        }
    }

    public void pageBeginRender(PageEvent event) {
        User user = getUser();
        if (user == null) {
            user = new User();
            user.setSipPassword(RandomStringUtils.randomAlphanumeric(SIP_PASSWORD_LEN));
            setUser(user);
        }
    }
}
