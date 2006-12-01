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

import java.util.ArrayList;
import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.PageCallback;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.speeddial.SpeedDialPage;
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
        page.setRings(null);
        return page;
    }

    public IPage editSpeedDial(IRequestCycle cycle, Integer userId) {
        SpeedDialPage page = (SpeedDialPage) cycle.getPage(SpeedDialPage.PAGE);
        page.setUserId(userId);
        return page;
    }

    public IPage editUser(IRequestCycle cycle, Integer userId) {
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);
        return page;
    }

    public IPage editSettings(IRequestCycle cycle, Integer userId, String path) {
        UserSettings page = (UserSettings) cycle.getPage(UserSettings.PAGE);
        page.setUserId(userId);
        page.setParentSettingName(path);
        return page;
    }

    public IPage editSupervisorPermission(IRequestCycle cycle, Integer userId) {
        SupervisorPermission page = (SupervisorPermission) cycle
                .getPage(SupervisorPermission.PAGE);
        page.setUserId(userId);
        page.setCallback(new PageCallback(ManageUsers.PAGE));
        return page;
    }

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setSettings(getUser().getSettings());
    }

    public Collection<Setting> getNavigationGroups() {
        Setting settings = getSettings();
        return getUserNavigationGroups(settings);
    }

    /**
     * We need to flatten user settings so the permissions show up on a higher level. We convert
     * tree that looks like this: <code>
     * - permission
     * -- application permissions
     * -- call handling 
     * - group 1
     * - group 2
     * </code>
     * 
     * Into tree that looks like this: <code>
     * - aplication permissions 
     * - call handling
     * - group 1
     * - group 2
     * </code>
     * 
     */
    public static Collection<Setting> getUserNavigationGroups(Setting settings) {
        Collection<Setting> result = new ArrayList<Setting>();
        for (Setting group : settings.getValues()) {
            if (group.getParent() != settings) {
                // only first level groups are interesting
                continue;
            }
            if (group.getName().equals("permission")) {
                result.addAll(group.getValues());
            } else {
                result.add(group);
            }
        }

        return result;
    }
}
