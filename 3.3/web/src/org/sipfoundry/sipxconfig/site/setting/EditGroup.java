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
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditGroup extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "EditGroup";

    public abstract String getResource();

    public abstract void setResource(String resourceId);

    public abstract Group getGroup();

    public abstract void setGroup(Group group);

    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract SettingDao getSettingContext();

    public abstract void setCallback(ICallback callback);

    public void pageBeginRender(PageEvent event_) {
        Group group = getGroup();
        if (group != null) {
            return;
        }
        Integer groupId = getGroupId();
        if (groupId != null) {
            group = getSettingContext().loadGroup(groupId);
        } else {
            group = new Group();
            group.setResource(getResource());
        }
        setGroup(group);
    }

    public void newGroup(String resourceId, String returnPage) {
        setResource(resourceId);
        setGroupId(null);
        setCallback(new PageCallback(returnPage));
    }

    public void editGroup(Integer groupId, String returnPage) {
        setResource(null);
        setGroupId(groupId);
        setCallback(new PageCallback(returnPage));
    }

    /*
     * If the input is valid, then save changes to the group.
     */
    public void apply(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Group group = getGroup();
        getSettingContext().saveGroup(group);
        setGroupId(group.getId());
    }
}
