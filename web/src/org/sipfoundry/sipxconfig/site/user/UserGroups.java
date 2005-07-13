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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.phone.PhoneModels;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

public abstract class UserGroups extends BasePage  implements PageRenderListener {
    
    public static final String PAGE = "UserGroups";
    
    public abstract void setGroups(List groups);
    
    public abstract List getGroups();
    
    public abstract CoreContext getCoreContext();
    
    public abstract SettingDao getSettingDao();
        
    public void addGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.newGroup("phone", PAGE);
        cycle.activate(page);
    }
    
    public Map getMemberCounts() {
        Map memberCount = getSettingDao().getGroupMemberCountIndexedByGroupId(User.class);
        
        return memberCount;
    }
    
    public void editPhoneGroup(IRequestCycle cycle) {
        PhoneModels page = (PhoneModels) cycle.getPage(PhoneModels.PAGE);
        Integer groupId = (Integer) TapestryUtils.assertParameter(Integer.class, cycle.getServiceParameters(), 0);
        page.setGroupId(groupId);
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event_) {
        CoreContext context = getCoreContext();
        setGroups(context.getUserGroups());
    }
}
