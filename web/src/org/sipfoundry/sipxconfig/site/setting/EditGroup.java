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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditGroup extends BasePage {
    
    public static final String PAGE = "EditGroup";
    
    public abstract void setReturnPage(String page);
    
    public abstract String getReturnPage();
    
    public abstract String getResource();

    public abstract Group getGroup();
    
    public abstract void setGroup(Group group);
    
    public abstract SettingDao getSettingContext();
    
    public void newGroup(String resourceId, String returnPage) {
        Group group = new Group();
        group.setResource(resourceId);
        setGroup(group);
        setReturnPage(returnPage);
    }
    
    public void editGroup(Integer groupId, String returnPage) {
        Group group = getSettingContext().loadGroup(groupId);
        setGroup(group);
        setReturnPage(returnPage);
    }
    
    public void apply(IRequestCycle cycle_) {
        save();
    }

    public void ok(IRequestCycle cycle) {
        save();
        cycle.activate(getReturnPage());
    }
    
    void save() {
        Group group = getGroup();
        getSettingContext().storeGroup(group);
    }

    public void cancel(IRequestCycle cycle) {        
        cycle.activate(getReturnPage());
    }
}
