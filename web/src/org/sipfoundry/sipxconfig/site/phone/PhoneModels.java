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
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

public abstract class PhoneModels extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "PhoneModels";
    
    public abstract void setGroupId(Integer id);
    
    public abstract Integer getGroupId();
    
    public abstract void setGroup(Group group);
    
    public abstract SettingDao getSettingContext();
    
    public void editGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        cycle.activate(page);
    }
    
    public void editPhoneDefaults(IRequestCycle cycle) {
        EditPhoneDefaults page = (EditPhoneDefaults) cycle.getPage(EditPhoneDefaults.PAGE);
        String factoryId = (String) TapestryUtils.assertParameter(String.class, cycle.getServiceParameters(), 0);
        page.editPhoneSettings(factoryId, getGroupId());
        cycle.activate(page);
    }

    public void pageBeginRender(PageEvent event_) {
        Group g = getSettingContext().loadGroup(getGroupId());
        setGroup(g);
    }    
}
