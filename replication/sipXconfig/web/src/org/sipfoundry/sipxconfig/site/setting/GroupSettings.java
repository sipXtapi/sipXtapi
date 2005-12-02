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
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupSettings extends BasePage implements PageRenderListener {

    public static final String PAGE = "GroupSettings";

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer id);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public abstract SettingDao getSettingDao();
    
    public abstract void setSettings(Setting setting);
    
    public abstract Setting getSettings();
    
    public abstract void setReturnPage(String returnPage);
    
    public abstract String getReturnPage();
    
    public void editGroupName(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        cycle.activate(page);
    }

    public void editGroup(Integer groupId, Setting settings, String returnPage) {
        setSettings(settings);
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    public void pageBeginRender(PageEvent event_) {
        Group group = getGroup();
        if (group != null) {
            return;
        }
        
        group  = getSettingDao().loadGroup(getGroupId());
        setGroup(group);
        String currentSettingName = getParentSettingName();
        if (currentSettingName == null) {
            Setting first = (Setting) getSettings().getValues().iterator().next();
            currentSettingName = first.getName();
        }
        Setting settings = getSettings().copy();
        group.decorate(settings);
        Setting parent = settings.getSetting(currentSettingName);
        setParentSetting(parent);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(getReturnPage());
    }

    public void apply(IRequestCycle cycle_) {
        SettingDao dao = getSettingDao();
        dao.saveGroup(getGroup());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(getReturnPage());
    }
}
