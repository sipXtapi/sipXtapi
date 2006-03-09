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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;

public abstract class UserNavigation extends BaseComponent {

    /** REQUIRED PARAMETER */
    public abstract void setUser(User user);

    public abstract User getUser();

    public abstract void setSettings(Setting settings);

    public abstract Setting getSettings();

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();

    public abstract CoreContext getCoreContext();

    public IPage editCallForwarding(IRequestCycle cycle, Integer userId) {
        UserCallForwarding page = (UserCallForwarding) cycle.getPage(UserCallForwarding.PAGE);
        page.setUserId(userId);
        return page;
    }

    public IPage editUser(IRequestCycle cycle, Integer userId) {
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);
        return page;
    }

    public IPage editSettings(IRequestCycle cycle, Integer userId, String section) {
        UserSettings page = (UserSettings) cycle.getPage(UserSettings.PAGE);
        page.setUserId(userId);
        page.setParentSettingName(section);
        return page;
    }

    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] {
            getUser().getId(), getCurrentSetting().getName()
        };
    }

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setSettings(getUser().getSettings());
    }
}
