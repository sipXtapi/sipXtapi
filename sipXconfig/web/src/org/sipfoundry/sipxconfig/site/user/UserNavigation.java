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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.common.NavigationWithSettings;
import org.sipfoundry.sipxconfig.site.speeddial.SpeedDialPage;
import org.sipfoundry.sipxconfig.site.user_portal.UserCallForwarding;

@ComponentClass(allowBody = true, allowInformalParameters = false)
public abstract class UserNavigation extends NavigationWithSettings {

    @Parameter(required = true)
    public abstract void setUser(User user);

    public abstract User getUser();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectPage(value = SpeedDialPage.PAGE)
    public abstract SpeedDialPage getSpeedDialPage();

    @InjectPage(value = UserCallForwarding.PAGE)
    public abstract UserCallForwarding getUserCallForwardingPage();

    @InjectPage(value = EditUser.PAGE)
    public abstract EditUser getEditUserPage();

    @InjectPage(value = UserSettings.PAGE)
    public abstract UserSettings getUserSettingsPage();

    @InjectPage(value = SupervisorPermission.PAGE)
    public abstract SupervisorPermission getSupervisorPermissionPage();

    @InjectPage(value = UserPhones.PAGE)
    public abstract UserPhones getUserPhonesPage();

    public IPage editCallForwarding(Integer userId) {
        UserCallForwarding page = getUserCallForwardingPage();
        page.setUserId(userId);
        page.setRings(null);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editSpeedDial(Integer userId) {
        SpeedDialPage page = getSpeedDialPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editUser(Integer userId) {
        EditUser page = getEditUserPage();
        page.setUserId(userId);
        return page;
    }

    public IPage editSettings(Integer userId, String path) {
        UserSettings page = getUserSettingsPage();
        page.setUserId(userId);
        page.setParentSettingName(path);
        return page;
    }

    public IPage editSupervisorPermission(Integer userId) {
        SupervisorPermission page = getSupervisorPermissionPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public IPage editUserPhones(Integer userId) {
        UserPhones page = getUserPhonesPage();
        page.setUserId(userId);
        page.setReturnPage(ManageUsers.PAGE);
        return page;
    }

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        Setting settings = getUser().getSettings();
        setSettings(settings);
        setMessageSource(settings.getMessageSource());
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
