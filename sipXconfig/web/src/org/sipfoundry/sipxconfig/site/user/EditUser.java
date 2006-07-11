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

import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

public abstract class EditUser extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "EditUser";

    public abstract CoreContext getCoreContext();
    
    public abstract SettingDao getSettingDao();

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public void commit() {
        if (TapestryUtils.isValid(this)) {
            User user = getUser();
            EditGroup.saveGroups(getSettingDao(), user.getGroups());
            getCoreContext().saveUser(user);
        }
    }

    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);
        }

        // If no callback has been given, then navigate back to Manage Users on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }
}
