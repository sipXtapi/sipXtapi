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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.Server;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class ServerSettings extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "ServerSettings";

    public abstract Server getServer();

    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public void pageBeginRender(PageEvent event_) {
        Server server = getServer();
        Setting root = server.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }
        
    public void editSettings(Integer serverId, String settingPath) {
        setParentSettingName(settingPath);
    }

    public IPage apply(IRequestCycle cycle) {
        getServer().applySettings();
        RestartReminder restartPage = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        restartPage.setNextPage(PAGE);
        return restartPage;
    }

    public void cancel() {
        getServer().resetSettings();
    }
}
