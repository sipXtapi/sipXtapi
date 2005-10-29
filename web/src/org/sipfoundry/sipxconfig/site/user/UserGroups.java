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

import java.util.List;
import java.util.Map;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;

public abstract class UserGroups extends BasePage  implements PageRenderListener {
    
    public static final String PAGE = "UserGroups";
    
    public abstract void setGroups(List groups);
    
    public abstract List getGroups();
    
    public abstract CoreManager getCoreContext();
    
    public abstract SettingDao getSettingContext();
        
    public void addGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.newGroup(User.GROUP_RESOURCE_ID, PAGE);
        cycle.activate(page);
    }
    
    public Map getMemberCounts() {
        Map memberCount = getSettingContext().getGroupMemberCountIndexedByGroupId(User.class);
        
        return memberCount;
    }
    
    public void editUserGroup(IRequestCycle cycle) {
        GroupSettings page = (GroupSettings) cycle.getPage(GroupSettings.PAGE);
        Integer groupId = (Integer) TapestryUtils.assertParameter(Integer.class, cycle.getServiceParameters(), 0);
        Setting model = getCoreContext().getUserSettingsModel();
        page.editGroup(groupId, model, PAGE);
        cycle.activate(page);
    }
    
    public void showGroupMembers(IRequestCycle cycle) {
        ManageUsers page = (ManageUsers) cycle.getPage(ManageUsers.PAGE);
        Integer groupId = (Integer) TapestryUtils.assertParameter(Integer.class, cycle.getServiceParameters(), 0);
        page.setGroupId(groupId);
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event_) {
        CoreManager context = getCoreContext();
        setGroups(context.getGroups());
    }
}
